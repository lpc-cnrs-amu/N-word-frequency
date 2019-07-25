# -*- coding: utf-8 -*-
"""
Created on Wed Jul 24 09:44:28 2019

@author: Marjorie
"""

import os
import sys
import re
import csv
from concurrent import futures
import threading
import pandas as pd

 
def calcul_frequences(filename, somme_match_count, somme_volume_count):
    df = pd.read_csv(filename, sep=';', encoding='utf-8-sig')
    df['freq match count'] = df['somme match count'] / somme_match_count
    df['freq volume count'] = df['somme volume count'] / somme_volume_count
    
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



langage = 'fre'
version = '20120701'
nb_ngram = '4'
indexes = []

#take all the indexes
if not indexes or (indexes and 'all' in indexes):
    indexes = []
    with open('files/indexes.txt', 'r') as f:
        for line in f:
            indexes.append(line.strip())
        
# append a | between each indexes for the regex 
strIndexes = '|'.join(map(str, indexes))

# take all the files we have
directory = os.getcwd()+'/results/freqNword/4grams/'
filenames = [f for f in os.listdir(directory) if re.match(r''+langage+'-all-'+\
             nb_ngram+'gram-'+version+'-('+strIndexes+').csv', f)]


tmp = pd.read_csv('files/nb-total-occurences-'+langage+'-'+nb_ngram+'gram-'+version+'.csv', sep=';')
somme_match_count = tmp['total match count'][0]
somme_volume_count = tmp['total volume count'][0]

    


with futures.ThreadPoolExecutor() as executor:
    
    future_to_url = {executor.submit(calcul_frequences, directory+filename,\
                                     somme_match_count, somme_volume_count): filename for filename in filenames}

    for future in futures.as_completed(future_to_url):
        url = future_to_url[future]
        try:
            print('finish :', url)
        except Exception as exc:
            print('%r generated an exception: %s' % (url, exc))
    

   
