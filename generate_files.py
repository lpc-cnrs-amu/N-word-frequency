# -*- coding: utf-8 -*-
"""
Created on Wed Jul 17 11:02:56 2019

@author: Marjorie
"""

import io
import time
import sys
import re
import requests
import zlib
import csv
import queue
import threading



class StreamInterruptionError(Exception):
    """Raised when a data stream ends before the end of the file"""
    def __init__(self, message):
        self.message = message     

exitFlag = 0
class myThread (threading.Thread):
    def __init__(self, threadID, name, q, session):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.q = q
        self.session = session
    def run(self):
        print("Starting " + self.name)
        write_final_files(self.name, self.q, self.session)
        print("Exiting " + self.name)
        
  
         
def write_final_files(threadName, q, session, tags_regex=r'(_NOUN|_VERB|_ADJ|_ADV|_PRON|_DET|_ADP|_NUM|_CONJ|_PRT)',\
                      nb_ngrams=4, chunk_size=1024 ** 2):
    session = requests.Session()
    
    while not exitFlag:
        queueLock.acquire()
        if not workQueue.empty():
            url = q.get()
            queueLock.release()            
            request = session.get(url, stream=True)
            print("%s - url: %s" % (threadName, url))
        
            if request.status_code != 200:
                sys.stderr.write("WARNING! your last request has failed -- Code {}\nURL = {}".format(
                        request.status_code, url))
                sys.stderr.flush()
            else:
                filename = url.replace('http://storage.googleapis.com/books/ngrams/books/googlebooks-', '')
                filename = filename.replace('.gz', '')
                filename_correct = filename + '_correct.csv'
                filename_incorrect = filename + '_incorrect.csv'                
                
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
                    print("\t\trequest ok - ", threadName)
        
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
        else:
            queueLock.release()
        time.sleep(1)
        
        
        
        
        
        

def write_raw_files(list_url, tags_regex, nb_ngrams, chunk_size=1024 ** 2):
    session = requests.Session()
    
    for url in list_url:
        request = session.get(url, stream=True)
        print("url: ", url)
        
        if request.status_code != 200:
            sys.stderr.write("WARNING! your last request has failed -- Code {}\nURL = {}".format(
                    request.status_code, url))
            sys.stderr.flush()
        else:
            filename = url.replace('http://storage.googleapis.com/books/ngrams/books/googlebooks-', '')
            filename = filename.replace('.gz', '')
            filename_correct = filename + '_correct.raw.csv'
            filename_incorrect = filename + '_incorrect.raw.csv'                
            
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
                print("\t\trequest ok")
    
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


langage = 'fre'
nb_ngram = '4'
version = '20120701'
year = 1970
indexes = ['wh', 'jn', 'vn', 'yo', 'fn','xx','ul','od','tp','oi','ox','hn','bn','we','ze']
final_file = False


#take all the indexes
if not indexes or (indexes and 'all' in indexes):
    indexes = []
    with open('indexes.txt', 'r') as f:
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

print(list_url)
print(len(list_url))
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


threadList = ["Thread-1", "Thread-2", "Thread-3","Thread-4","Thread-5"]
queueLock = threading.Lock()
workQueue = queue.Queue(0)
threads = []
threadID = 1

# Create new threads
session = requests.Session()
for tName in threadList:
    thread = myThread(threadID, tName, workQueue, session)
    thread.start()
    threads.append(thread)
    threadID += 1

# Fill the queue
queueLock.acquire()
for url in list_url:
    workQueue.put(url)
queueLock.release()

# Wait for queue to empty
while not workQueue.empty():
   pass

# Notify threads it's time to exit
exitFlag = 1

# Wait for all threads to complete
for t in threads:
   t.join()
print("Exiting Main Thread")

