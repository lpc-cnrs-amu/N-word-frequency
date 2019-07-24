# -*- coding: utf-8 -*-
"""
Created on Wed Jul 24 09:44:28 2019

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
import pandas as pd


class StreamInterruptionError(Exception):
    """Raised when a data stream ends before the end of the file"""
    def __init__(self, message):
        self.message = message     

         
 
def get_total_number_words(url, session, is_version_2012, chunk_size=1024 ** 2):        
    with session.get(url, stream=True) as r:
        print("url: ", url)
        if r.status_code != 200:
            sys.stderr.write("WARNING! your last request has failed -- Code {}\nURL = {}".format(
                    r.status_code, url))
            sys.stderr.flush()
        else:  
            if is_version_2012:
                lines_separator = b'\t'
                data_separator = ','
            else:
                lines_separator = b'\n'
                data_separator = '\t'            
            print("\t\trequest ok")
    
            somme_match_count = 0
            somme_volume_count = 0
            cpt_lines = -1    
            last = b''
            
            for chunk in r.iter_content(chunk_size=chunk_size):
    
                lines = (last + chunk).split(lines_separator)
                lines, last = lines[:-1], lines[-1]
    
                    
                for line in lines:
                    cpt_lines += 1
                    if cpt_lines == 0:
                        continue
                    
                    line = line.decode('utf-8')
                    data = line.split(data_separator)
                    assert len(data) == 4
                    year = int(data[0])
                    count_match = int(data[1])
                    nb_volumes = int(data[3])
                        
                    if year>=1972:
                        print("line: [%s]" % line)
                        print(data)
                        somme_match_count += count_match
                        somme_volume_count += nb_volumes
    
                           
                if last:
                    raise StreamInterruptionError(
                        "Data stream ended on a non-empty line. This might be due "
                        "to temporary networking problems.")
    
            return somme_match_count, somme_volume_count
                        
                        

def calcul_frequences(filename, somme_match_count):
    df = pd.read_csv(filename, sep=';', encoding='utf-8-sig')
    df['freq match count'] = df['somme match count'] / somme_match_count
    
    filename = filename.replace('.csv', '-frequences.csv')
    df.to_csv(filename, index=False, na_rep='NaN', encoding='utf-8-sig', sep=";")

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


if __name__ == '__main__': 
    langage = 'fre'
    version = '20120701'
    
    regex = r'http://storage\.googleapis\.com/books/ngrams/books/googlebooks-'\
        +langage+'-all-totalcounts-'+version+'\.txt'
    
    # find all corresponding url
    list_url = []
    with open("files/google_url.txt", "r") as f:
        for line in f:
            if re.search(regex, line):
                list_url.append(line.strip())
    
    print(list_url)
    #print(len(list_url))
    if not list_url:
        print("No URL found")
        sys.exit()
        
    indexes = []
    
    #take all the indexes
    if not indexes or (indexes and 'all' in indexes):
        indexes = []
        with open('files/indexes.txt', 'r') as f:
            for line in f:
                indexes.append(line.strip())
            
    # append a | between each indexes for the regex 
    strIndexes = '|'.join(map(str, indexes))
    
    # take all the 1gram files we have
    directory = os.getcwd()+'/results/freqNword/'
    filenames = [f for f in os.listdir(directory) if re.match(r''+langage+'-all-1gram-'+version+'-('+strIndexes+').csv', f)]
    
    
    
    lang_to_somme_match_count = {} 
    lang_to_somme_volume_count = {}
    session = requests.Session()
    
    for url in list_url:
        if version=='20120701':
            somme_match_count, somme_volume_count = get_total_number_words(url, session, True, chunk_size=1024 ** 2)
        else:
            somme_match_count, somme_volume_count = get_total_number_words(url, session, False, chunk_size=1024 ** 2)
            
        if langage not in lang_to_somme_match_count:
            lang_to_somme_match_count[langage] = somme_match_count
        if langage not in lang_to_somme_volume_count:
            lang_to_somme_volume_count[langage] = somme_volume_count
        
        
        print("For ", url)
        print('\tnb total of words : ', lang_to_somme_match_count[langage])
        print('\tnb total of volume : ', lang_to_somme_volume_count[langage])
    
    for filename in filenames:
        print("Doing ", filename)
        calcul_frequences(directory+filename, somme_match_count)


