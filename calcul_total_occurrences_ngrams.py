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


lck = threading.Lock()
nb_total_match_count = 0
nb_total_volume_count = 0

         
 
def calculate_nb_total_match_count(filename):    
    global lck, nb_total_match_count, nb_total_volume_count
    
    df = pd.read_csv(filename, sep=';')
    total_match_count = df['somme match count'].sum()
    total_volume_count = df['somme volume count'].sum()
    
    lck.acquire()
    nb_total_match_count += total_match_count
    nb_total_volume_count += total_volume_count
    lck.release()
    
                        

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



with open("files/nb-total-occurences-" +langage+ "-" +nb_ngram+"gram-"+version+".csv", "w", encoding="utf-8-sig", newline='') as f:
    writer = csv.writer(f, delimiter=';', quotechar='"', \
                        quoting=csv.QUOTE_MINIMAL)
    writer.writerow(['lang', 'total match count', 'total volume count'])
    with futures.ThreadPoolExecutor() as executor:
        
        future_to_url = {executor.submit(calculate_nb_total_match_count, directory+filename): filename for filename in filenames}

        for future in futures.as_completed(future_to_url):
            url = future_to_url[future]
            try:
                print('finish :', url)
            except Exception as exc:
                print('%r generated an exception: %s' % (url, exc))
    
    writer.writerow([langage, nb_total_match_count, nb_total_volume_count])
    print("total match count: ", nb_total_match_count)
    print("total volume count: ", nb_total_volume_count)
    
