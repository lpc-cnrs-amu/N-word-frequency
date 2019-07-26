# -*- coding: utf-8 -*-
"""
Created on Fri Jul 26 14:21:31 2019

@author: Marjorie
"""

"""
Calculate the occurrences for each "tags-grams". For example :
    DET NOUN VERB ADJ
    NOUN VERB DET ADJ

all the occurrences are written in files/nb-total-occurences-per-tags-" +langage+ 
                                    "-" +str(nb_ngram)+"gram-"+version+".csv"
"""


import os
import sys
import re
import csv
import threading
from concurrent import futures


        
 
def pretreat_files(filename, list_forbidden_characters, nb_ngram=4):
    output_filename = filename.replace('.csv', '-pretreat.csv')
    
    with open(filename, "r", encoding="utf-8-sig", newline='') as f,\
    open(output_filename, "w", encoding="utf-8-sig", newline='') as output:
        reader = csv.reader(f, delimiter=';')
        
        writer = csv.writer(output, delimiter=';', quotechar='"', \
                            quoting=csv.QUOTE_MINIMAL)
        
        cpt_line = -1
        for line in reader:
            cpt_line += 1
            if cpt_line==0:
                writer.writerow(line)
            else:
                not_write = False
                
                for i in range(0, nb_ngram*2, 2):
                    if line[i] in list_forbidden_characters:
                        not_write = True
                        break
                
                if not_write:
                    continue
                else:
                    writer.writerow(line)
                   

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

list_forbidden_characters = [',','.','?','!','...',';',':','"']

langage = 'fre'
version = '20120701'
nb_ngram = 4
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
             str(nb_ngram)+'gram-'+version+'-('+strIndexes+')-frequences.csv', f)]

with futures.ThreadPoolExecutor() as executor:
    
    future_to_url = {executor.submit(pretreat_files, directory+filename,\
                                     list_forbidden_characters, nb_ngram): filename for filename in filenames}

    for future in futures.as_completed(future_to_url):
        url = future_to_url[future]
        try:
            print('finish :', url)
        except Exception as exc:
            print('%r generated an exception: %s' % (url, exc))
