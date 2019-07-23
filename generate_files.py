# -*- coding: utf-8 -*-
"""
Created on Wed Jul 17 11:02:56 2019

@author: Marjorie
"""

import os
import time
import sys
import re
import requests
import zlib
import csv
from concurrent import futures
import threading



class StreamInterruptionError(Exception):
    """Raised when a data stream ends before the end of the file"""
    def __init__(self, message):
        self.message = message     

        
         
def write_final_files(url, session, tags_regex=r'(_NOUN|_VERB|_ADJ|_ADV|_PRON|_DET|_ADP|_NUM|_CONJ|_PRT)',\
                      nb_ngrams=4, chunk_size=1024 ** 2):        
    request = session.get(url, stream=True)
    print("%s - url: %s" % (threading.current_thread().name, url))

    if request.status_code != 200:
        sys.stderr.write("WARNING! your last request has failed -- Code {}\nURL = {}".format(
                request.status_code, url))
        sys.stderr.flush()
    else:
        filename = url.replace('http://storage.googleapis.com/books/ngrams/books/googlebooks-', '')
        filename = filename.replace('.gz', '')
        filename_correct = os.path.join('results', 'freqNword', filename + '.csv')
        filename_incorrect = os.path.join('results', 'ignored_items', filename + '_ignored.csv')               
        
        with open(filename_correct, 'w', encoding="utf-8-sig", newline='') as \
        output_ok, open(filename_incorrect, 'w', encoding="utf-8-sig", newline='') as \
        output_incorrect:
            ok_writer = csv.writer(output_ok, delimiter=';', quotechar='"', \
                                   quoting=csv.QUOTE_MINIMAL)
            incorrect_writer = csv.writer(output_incorrect, delimiter=';', \
                                          quotechar='"', quoting=csv.QUOTE_MINIMAL)
            
            header = ['gram 1', 'tag 1', 'gram 2', 'tag 2', \
                      'gram 3', 'tag 3', 'gram 4', 'tag 4', \
                      'nb year', 'somme match count', \
                      'somme volume count', \
                      'mean_pondere_count_match', 'mean_pondere_volume_match', \
                      'year max', 'year min',\
                      'match count max', 'match count min',\
                      'volume match max', 'volume match min',\
                      'number of tags']
            ok_writer.writerow(header)
            incorrect_writer.writerow(header)                
            
            decompression = zlib.decompressobj(32 + zlib.MAX_WBITS)
            
            last = b''
            compressed_chunks = request.iter_content(chunk_size=chunk_size)
            print("\t\trequest ok - ", threading.current_thread().name)

            precedent_ngram = ""
            somme_count_match = 0
            somme_year = 0
            somme_volume_match = 0
            mean_pondere_count_match = 0
            mean_pondere_volume_match = 0
            year_max = 0
            year_min = 3000
            count_match_max = 0
            count_match_min = 3000
            volume_match_max = 0
            volume_match_min = 3000
            
            for i, compressed_chunk in enumerate(compressed_chunks):
                #print("\t\t{} num chunck: {}".format(filename, i))
                chunk = decompression.decompress(compressed_chunk)

                lines = (last + chunk).split(b'\n')
                lines, last = lines[:-1], lines[-1]

                
                for line in lines:
                    line = line.decode('utf-8')
                    data = line.split('\t')
                    assert len(data) == 4
                    ngram = data[0]
                    year = int(data[1])
                    count_match = int(data[2])
                    volume_match = int(data[3])
                    
                    if year>=1972:
                                     
                        #print("\tcpt_line = ", cpt_line)
                        #print("\ti = ", i)
                        # on rencontre un nouveau ngram donc on save l'ancien sauf si c'est le 1er du fichier
                        if ngram != precedent_ngram and precedent_ngram != "":
                            
                            #nb de tags
                            nb_tags = re.findall(tags_regex, precedent_ngram)

                            #contient les mots attachés à leur pos : donc len(grams)=nb_grams
                            grams = precedent_ngram.split()
                                   
                            row = []
                            words_tags = []
                            print_correct = True
                            
                            #pour chaque entité WORD_TAG
                            for word in grams:
                                words_tags = word.split('_')
                                #si on en a 2 alors le mot est taggé
                                if len(words_tags) == 2:
                                    row.extend([words_tags[0].lower(), words_tags[1]])
                                #sinon il ne l'est pas
                                else:
                                    row.extend([word.lower(), ''])
                                    print_correct = False                                
                            
                            row.extend([somme_year, somme_count_match,\
                                        somme_volume_match, round(mean_pondere_count_match/somme_count_match, 2), \
                                        round(mean_pondere_volume_match/somme_volume_match, 2),\
                                        year_max, year_min,\
                                        count_match_max, count_match_min,\
                                        volume_match_max, volume_match_min,\
                                        len(nb_tags)]) 
                
                            if len(nb_tags) != nb_ngrams or not print_correct \
                            or len(grams) != nb_ngrams:
                                incorrect_writer.writerow(row)
                            else:
                                ok_writer.writerow(row)
                            
                            somme_count_match = count_match
                            somme_year = 1
                            somme_volume_match = volume_match
                            mean_pondere_count_match = year*count_match 
                            mean_pondere_volume_match = year*volume_match 
                            year_max = 0
                            year_min = 3000
                            count_match_max = 0
                            count_match_min = 3000
                            volume_match_max = 0
                            volume_match_min = 3000
                        else:
                            #print("on écrit pas : ", ngram)
                            somme_count_match += count_match
                            somme_year += 1
                            somme_volume_match += volume_match
                            mean_pondere_count_match = mean_pondere_count_match + year*count_match 
                            mean_pondere_volume_match = mean_pondere_volume_match + year*volume_match                             

                        if year > year_max:
                            year_max = year
                        if year < year_min:
                            year_min = year

                        if count_match > count_match_max:
                            count_match_max = count_match
                        if count_match < count_match_min:
                            count_match_min = count_match
                            
                        if volume_match > volume_match_max:
                            volume_match_max = volume_match
                        if volume_match < volume_match_min:
                            volume_match_min = volume_match 
                            
                        precedent_ngram = ngram
     
            if last:
                raise StreamInterruptionError(
                    "Data stream ended on a non-empty line. This might be due "
                    "to temporary networking problems.")
            if year>=1972:
                     
                if year > year_max:
                    year_max = year
                if year < year_min:
                    year_min = year

                if count_match > count_match_max:
                    count_match_max = count_match
                if count_match < count_match_min:
                    count_match_min = count_match
                    
                if volume_match > volume_match_max:
                    volume_match_max = volume_match
                if volume_match < volume_match_min:
                    volume_match_min = volume_match  
                #nb de tags
                nb_tags = re.findall(tags_regex, precedent_ngram)

                #contient les mots attachés à leur pos : donc len(grams)=nb_grams
                grams = precedent_ngram.split()
                       
                row = []
                words_tags = []
                print_correct = True
                
                #pour chaque entité WORD_TAG
                for word in grams:
                    words_tags = word.split('_')
                    #si on en a 2 alors le mot est taggé
                    if len(words_tags) == 2:
                        row.extend([words_tags[0].lower(), words_tags[1]])
                    #sinon il ne l'est pas
                    else:
                        row.extend([word.lower(), ''])
                        print_correct = False                                
                
                row.extend([somme_year, somme_count_match,\
                            somme_volume_match, round(mean_pondere_count_match/somme_count_match, 2), \
                            round(mean_pondere_volume_match/somme_volume_match, 2),\
                            year_max, year_min,\
                            count_match_max, count_match_min,\
                            volume_match_max, volume_match_min,\
                            len(nb_tags)]) 
    
                if len(nb_tags) != nb_ngrams or not print_correct \
                or len(grams) != nb_ngrams:
                    incorrect_writer.writerow(row)
                else:
                    ok_writer.writerow(row)
            return "\tfini - "+ threading.current_thread().name
        
        
        
        
        
        

def write_raw_files(url, session, tags_regex=r'(_NOUN|_VERB|_ADJ|_ADV|_PRON|_DET|_ADP|_NUM|_CONJ|_PRT)',\
                    nb_ngrams=4, chunk_size=1024 ** 2):
    
    request = session.get(url, stream=True)
    print("%s - url: %s" % (threading.current_thread().name, url))
        
    if request.status_code != 200:
        sys.stderr.write("WARNING! your last request has failed -- Code {}\nURL = {}".format(
                request.status_code, url))
        sys.stderr.flush()
    else:
        filename = url.replace('http://storage.googleapis.com/books/ngrams/books/googlebooks-', '')
        filename = filename.replace('.gz', '')
        filename_correct = os.path.join('results', 'raw_data', 'freqNword', filename + '.raw.csv')
        filename_incorrect = os.path.join('results', 'raw_data', 'ignored_items', filename + '_ignored.raw.csv')               
        
        with open(filename_correct, 'w', encoding="utf-8-sig", newline='') as \
        output_ok, open(filename_incorrect, 'w', encoding="utf-8-sig", newline='') as \
        output_incorrect:
            ok_writer = csv.writer(output_ok, delimiter=';', quotechar='"', \
                                   quoting=csv.QUOTE_MINIMAL)
            incorrect_writer = csv.writer(output_incorrect, delimiter=';', \
                                          quotechar='"', quoting=csv.QUOTE_MINIMAL)
            
            header = ['gram 1', 'tag 1', 'gram 2', 'tag 2', \
                      'gram 3', 'tag 3', 'gram 4', 'tag 4', \
                      'year', 'nb year', 'match count', 'somme match count', \
                      'volume count', 'somme volume count', \
                      'mean_pondere_count_match', 'mean_pondere_volume_match', \
                      'year max', 'year min',\
                      'match count max', 'match count min',\
                      'volume match max', 'volume match min',\
                      'number of tags']
            ok_writer.writerow(header)
            incorrect_writer.writerow(header)                
            
            decompression = zlib.decompressobj(32 + zlib.MAX_WBITS)
            
            last = b''
            compressed_chunks = request.iter_content(chunk_size=chunk_size)
            print("\t\trequest ok - ", threading.current_thread().name)

            precedent_ngram = ""
            somme_count_match = 0
            somme_year = 0
            somme_volume_match = 0
            mean_pondere_count_match = 0
            mean_pondere_volume_match = 0
            year_max = 0
            year_min = 3000
            count_match_max = 0
            count_match_min = 3000
            volume_match_max = 0
            volume_match_min = 3000
            
            
            for i, compressed_chunk in enumerate(compressed_chunks):
                chunk = decompression.decompress(compressed_chunk)

                lines = (last + chunk).split(b'\n')
                lines, last = lines[:-1], lines[-1]

                for line in lines:
                    line = line.decode('utf-8')
                    data = line.split('\t')
                    assert len(data) == 4
                    ngram = data[0]
                    year = int(data[1])
                    count_match = int(data[2])
                    volume_match = int(data[3])
                    
                    
                    if year>=1972:
                        #nb de tags
                        nb_tags = re.findall(tags_regex, ngram)

                        #contient les mots attachés à leur pos : donc len(grams)=nb_grams
                        grams = ngram.split()
                               
                        row = []
                        words_tags = []
                        print_correct = True
                        
                        #pour chaque entité WORD_TAG
                        for word in grams:
                            words_tags = word.split('_')
                            #si on en a 2 alors le mot est taggé
                            if len(words_tags) == 2:
                                row.extend([words_tags[0].lower(), words_tags[1]])
                            #sinon il ne l'est pas
                            else:
                                row.extend([word.lower(), ''])
                                print_correct = False
                                
                                
                        #somme des count_match, year, volume_count
                        if ngram != precedent_ngram:
                            somme_count_match = count_match
                            somme_year = 1
                            somme_volume_match = volume_match
                            mean_pondere_count_match = year*count_match 
                            mean_pondere_volume_match = year*volume_match 
                            year_max = 0
                            year_min = 3000
                            count_match_max = 0
                            count_match_min = 3000
                            volume_match_max = 0
                            volume_match_min = 3000
                        else:
                            somme_count_match += count_match
                            somme_year += 1
                            somme_volume_match += volume_match
                            mean_pondere_count_match = mean_pondere_count_match + year*count_match 
                            mean_pondere_volume_match = mean_pondere_volume_match + year*volume_match                             
                                
                        
                        if year > year_max:
                            year_max = year
                        if year < year_min:
                            year_min = year

                        if count_match > count_match_max:
                            count_match_max = count_match
                        if count_match < count_match_min:
                            count_match_min = count_match
                            
                        if volume_match > volume_match_max:
                            volume_match_max = volume_match
                        if volume_match < volume_match_min:
                            volume_match_min = volume_match
                            
                        row.extend([year, somme_year, count_match, somme_count_match,\
                                    volume_match, somme_volume_match, round(mean_pondere_count_match/somme_count_match, 2), \
                                    round(mean_pondere_volume_match/somme_volume_match, 2),\
                                    year_max, year_min,\
                                    count_match_max, count_match_min,\
                                    volume_match_max, volume_match_min,\
                                    len(nb_tags)])     
             
                        precedent_ngram = ngram
                        
                        if len(nb_tags) != nb_ngrams or not print_correct \
                        or len(grams) != nb_ngrams:
                            incorrect_writer.writerow(row)
                        else:
                            ok_writer.writerow(row)
                        
            
            if last:
                raise StreamInterruptionError(
                    "Data stream ended on a non-empty line. This might be due "
                    "to temporary networking problems.")
            if year>=1972:
                #nb de tags
                nb_tags = re.findall(tags_regex, ngram)

                #contient les mots attachés à leur pos : donc len(grams)=nb_grams
                grams = ngram.split()
                       
                row = []
                words_tags = []
                print_correct = True
                
                #pour chaque entité WORD_TAG
                for word in grams:
                    words_tags = word.split('_')
                    #si on en a 2 alors le mot est taggé
                    if len(words_tags) == 2:
                        row.extend([words_tags[0].lower(), words_tags[1]])
                    #sinon il ne l'est pas
                    else:
                        row.extend([word.lower(), ''])
                        print_correct = False
                        
                        
                #somme des count_match, year, volume_count
                somme_count_match += count_match
                somme_year += 1
                somme_volume_match += volume_match
                mean_pondere_count_match = mean_pondere_count_match + year*count_match 
                mean_pondere_volume_match = mean_pondere_volume_match + year*volume_match                             
                        
                
                if year > year_max:
                    year_max = year
                if year < year_min:
                    year_min = year

                if count_match > count_match_max:
                    count_match_max = count_match
                if count_match < count_match_min:
                    count_match_min = count_match
                    
                if volume_match > volume_match_max:
                    volume_match_max = volume_match
                if volume_match < volume_match_min:
                    volume_match_min = volume_match
                    
                row.extend([year, somme_year, count_match, somme_count_match,\
                            volume_match, somme_volume_match, round(mean_pondere_count_match/somme_count_match, 2), \
                            round(mean_pondere_volume_match/somme_volume_match, 2),\
                            year_max, year_min,\
                            count_match_max, count_match_min,\
                            volume_match_max, volume_match_min,\
                            len(nb_tags)])     
     
                
                if len(nb_tags) != nb_ngrams or not print_correct \
                or len(grams) != nb_ngrams:
                    incorrect_writer.writerow(row)
                else:
                    ok_writer.writerow(row)
            return "\tfini - "+ threading.current_thread().name
                        
                        


            

"""
if len(sys.argv) != :
    print("Usage : {} -l lang -n nb_ngram -v version -y [year] -i [index]".format(sys.argv[0]))
    sys.exit
    

langage = sys.argv[1]
nb_ngram = sys.argv[2]
version = sys.argv[3]
year = list(sys.argv[4])
indexes = list(sys.argv[5])
"""

#create explicitly "results" directory structure if not exist
if not os.path.exists(os.path.join('results')):
    os.makedirs(os.path.join('results'))
    
if not os.path.exists(os.path.join('results', 'freqNword')):
    os.makedirs(os.path.join('results', 'freqNword'))
    
if not os.path.exists(os.path.join('results', 'ignored_items')):
    os.makedirs(os.path.join('results', 'ignored_items'))
    
if not os.path.exists(os.path.join('results', 'raw_data', 'freqNword')):    
    os.makedirs(os.path.join('results', 'raw_data', 'freqNword'))
    
if not os.path.exists(os.path.join('results', 'raw_data', 'ignored_items')):
    os.makedirs(os.path.join('results', 'raw_data', 'ignored_items'))

    
    

langage = 'fre'
nb_ngram = '4'
version = '20120701'
year = 1970
indexes = []
final_file = True


#take all the indexes
if not indexes or (indexes and 'all' in indexes):
    indexes = []
    with open('files/indexes.txt', 'r') as f:
        for line in f:
            indexes.append(line.strip())
        
        
# append a | between each indexes for the regex 
strIndexes = '|'.join(map(str, indexes))


# only with gz file
regex = r'http://storage\.googleapis\.com/books/ngrams/books/googlebooks-'\
    +langage+'-all-'+nb_ngram+'gram-'+version+'-('+ strIndexes +')\.gz'


# find all corresponding url
list_url = []
with open("files/google_url.txt") as f:
    for line in f:
        if re.search(regex, line):
            list_url.append(line.strip())

#print(list_url)
#print(len(list_url))
if not list_url:
    print("No URL found")
    sys.exit()


tags_regex = r'(_NOUN|_VERB|_ADJ|_ADV|_PRON|_DET|_ADP|_NUM|_CONJ|_PRT)'

"""
if final_file:
    write_final_files(list_url, tags_regex, int(nb_ngram))
else:
    write_raw_files(list_url, tags_regex, int(nb_ngram))
"""
session = requests.Session()
with futures.ThreadPoolExecutor() as executor:
    if final_file:
        future_to_url = {executor.submit(write_final_files, url, session): url for url in list_url}
    else:
        future_to_url = {executor.submit(write_raw_files, url, session): url for url in list_url}
    for future in futures.as_completed(future_to_url):
        url = future_to_url[future]
        try:
            print(future.result())
        except Exception as exc:
            print('%r generated an exception: %s' % (url, exc))


#threading.current_thread().name