#!/usr/bin
# -*- coding: utf-8 -*-
"""
Created on Wed Jul 17 11:02:56 2019

@author: Marjorie
"""

import os
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

         
def write_final_files(url, session, header, list_forbidden_characters=[',','.','?','!','...',';',':','"'],
                      tags_regex=r'(_NOUN|_VERB|_ADJ|_ADV|_PRON|_DET|_ADP|_NUM|_CONJ|_PRT)',
                      nb_ngrams=4, chunk_size=1024 ** 2):        
    request = session.get(url, stream=True)

    if request.status_code != 200:
        sys.stderr.write("WARNING! your last request has failed -- Code {}\nURL = {}".format(
                request.status_code, url))
        sys.stderr.flush()
    else:
        filename = url.replace(
                'http://storage.googleapis.com/books/ngrams/books/googlebooks-', '')
        filename = filename.replace('.gz', '')
        filename_correct = os.path.join('results', 'freqNword', filename + '.csv')               
        
        with open(filename_correct, 'w', encoding="utf-8-sig", newline='') as output_ok:
            
            ok_writer = csv.writer(output_ok, delimiter=';', 
                                   quotechar='"', quoting=csv.QUOTE_MINIMAL)
            ok_writer.writerow(header)    
            print("%s - url: %s" % (threading.current_thread().name, url))
            
            decompression = zlib.decompressobj(32 + zlib.MAX_WBITS)
            
            last = b''
            compressed_chunks = request.iter_content(chunk_size=chunk_size)
            
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
                                #si on en a 2 alors le mot est bien taggé
                                if len(words_tags) == 2 and words_tags[0] not in list_forbidden_characters:
                                    row.extend([words_tags[0], words_tags[1]])
                                #sinon il ne l'est pas
                                else:
                                    print_correct = False
                                    break
                            
                            if len(nb_tags) == nb_ngrams and print_correct and len(grams) == nb_ngrams:
                                row.extend([somme_year, somme_count_match,
                                            somme_volume_match, 
                                            round(mean_pondere_count_match/somme_count_match, 2),
                                            round(mean_pondere_volume_match/somme_volume_match, 2),
                                            year_max, year_min,
                                            count_match_max, count_match_min,
                                            volume_match_max, volume_match_min,
                                            len(nb_tags)]) 
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
                    if len(words_tags) == 2 and words_tags[0] not in list_forbidden_characters:
                        row.extend([words_tags[0], words_tags[1]])
                    #sinon il ne l'est pas
                    else:
                        print_correct = False
                        break                             
                if len(nb_tags) == nb_ngrams and print_correct and len(grams) == nb_ngrams:
                    row.extend([somme_year, somme_count_match,\
                                somme_volume_match, round(mean_pondere_count_match/somme_count_match, 2), \
                                round(mean_pondere_volume_match/somme_volume_match, 2),\
                                year_max, year_min,\
                                count_match_max, count_match_min,\
                                volume_match_max, volume_match_min,\
                                len(nb_tags)]) 
                    ok_writer.writerow(row)
                    
            print('finish :', url)
            return [url, threading.current_thread().name]


            

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

    
    

langage = 'eng'
nb_ngram = 4
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
    +langage+'-all-'+str(nb_ngram)+'gram-'+version+'-('+ strIndexes +')\.gz'


# find all corresponding url and write them in a csv file
list_url = []
with open("files/google_url.txt", 'r') as f, open('files/url_to_read.csv', 'w', \
         encoding="utf-8-sig", newline='') as url_files:
    writer = csv.writer(url_files, delimiter=';', quotechar='"', \
                           quoting=csv.QUOTE_MINIMAL)
    writer.writerow(['url to read'])
    for line in f:
        if re.search(regex, line):
            list_url.append(line.strip())
            writer.writerow([line.strip()])
            

if not list_url:
    print("No URL found")
    sys.exit()
else:    
    print("URL found :\n", list_url)


header = []
for i in range(1, nb_ngram+1):
    header.extend(['gram '+str(i), 'tag '+str(i)])
header.extend(['nb year', 'somme match count', \
               'somme volume count', 'mean_pondere_count_match',\
               'mean_pondere_volume_match', 'year max', 'year min',\
               'match count max', 'match count min',\
               'volume match max', 'volume match min', 'number of tags'])    


session = requests.Session()
with open("files/url_ok2.csv", "w", encoding="utf-8-sig", newline='') as f:
    writer = csv.writer(f, delimiter=';', quotechar='"', \
                        quoting=csv.QUOTE_MINIMAL)
    writer.writerow(['url read', 'thread name'])
    
    with futures.ThreadPoolExecutor() as executor:
        future_to_url = {executor.submit(write_final_files, 
                                         url, session, header, 
                                         nb_ngrams=nb_ngram): url for url in list_url}
        for future in futures.as_completed(future_to_url):
            url = future_to_url[future]
            try:
                writer.writerow(future.result())
            except Exception as exc:
                print('%r generated an exception: %s' % (url, exc))