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
import collections
import csv


Record = collections.namedtuple('Record', 'ngram year match_count volume_count')

class StreamInterruptionError(Exception):
    """Raised when a data stream ends before the end of the file"""
    def __init__(self, message):
        self.message = message     



def readlines_url(list_url, tags_regex, nb_ngrams, chunk_size=1024 ** 2):
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
            
            with open(filename+'_correct.raw.csv', 'w', encoding="utf-8-sig", newline='') as \
            output_ok, open(filename+'_incorrect.raw.csv', 'w', encoding="utf-8-sig", newline='') as \
            output_incorrect:
                ok_writer = csv.writer(output_ok, delimiter=';', quotechar='"', \
                                       quoting=csv.QUOTE_MINIMAL)
                incorrect_writer = csv.writer(output_incorrect, delimiter=';', \
                                              quotechar='"', quoting=csv.QUOTE_MINIMAL)
                
                header = ['gram 1', 'tag 1', 'gram 2', 'tag 2', \
                          'gram 3', 'tag 3', 'gram 4', 'tag 4', \
                          'year', 'match count', \
                          'volume count', 'number of tags']
                ok_writer.writerow(header)
                incorrect_writer.writerow(header)                
                
                decompression = zlib.decompressobj(32 + zlib.MAX_WBITS)
                
                last = b''
                compressed_chunks = request.iter_content(chunk_size=chunk_size)
                print("\t\trequest ok")
    
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
                            
                            row.extend([year, count_match, \
                                        volume_match, len(nb_tags)])     
                  
                            if len(nb_tags) != nb_ngrams or not print_correct \
                            or len(grams) != nb_ngrams:
                                incorrect_writer.writerow(row)
                            else:
                                ok_writer.writerow(row)
                            
                
                if last:
                    raise StreamInterruptionError(
                        "Data stream ended on a non-empty line. This might be due "
                        "to temporary networking problems.")




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
indexes = ['az', 'ab']

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
with open("google_url.txt") as f:
    for line in f:
        if re.search(regex, line):
            list_url.append(line.strip())

print(list_url)
print(len(list_url))
if not list_url:
    print("No URL found")
    sys.exit()


tags_regex = r'(_NOUN|_VERB|_ADJ|_ADV|_PRON|_DET|_ADP|_NUM|_CONJ|_PRT)'
readlines_url(list_url, tags_regex, int(nb_ngram))


