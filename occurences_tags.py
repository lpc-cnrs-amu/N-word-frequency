# -*- coding: utf-8 -*-
"""
Created on Thu Jul 25 15:45:43 2019

@author: Marjorie
"""

import os
import sys
import re
import csv
from concurrent import futures
import threading


lck = threading.Lock()
tags_to_occurrences = {}
tags_to_year = {}
tags_to_volume = {}
tags_to_mean_pondere_count_match = {}
tags_to_mean_pondere_volume_match = {}
tags_to_year_max = {}
tags_to_year_min = {}
tags_to_match_count_max = {}
tags_to_match_count_min = {}
tags_to_volume_match_max = {}
tags_to_volume_match_min = {}
 
def calculate_nb_total_match_count(filename, nb_ngram=4):    
    global lck, tags_to_occurrences, tags_to_year, tags_to_volume,tags_to_mean_pondere_count_match,\
    tags_to_mean_pondere_volume_match,tags_to_year_max,tags_to_year_min,tags_to_match_count_max,\
    tags_to_match_count_min,tags_to_volume_match_max,tags_to_volume_match_min
    
    print("doing: ", filename)
    with open(filename, "r", encoding="utf-8-sig", newline='') as f:
        reader = csv.DictReader(f, delimiter=';', quotechar='"')
        cpt_line = -1
        
        for line in reader:
            cpt_line += 1
            if cpt_line==0:
                continue
            
            tags = []
            for i in range(1, nb_ngram*2, 2):
                tags.append(line[i])
            
            strTags = ' '.join(tags)
            
            lck.acquire()
            
            if strTags in tags_to_occurrences:
                tags_to_occurrences[strTags] += line['somme match count']
                tags_to_year[strTags] += line['nb year']
                tags_to_volume[strTags] += line['somme volume count']
                tags_to_mean_pondere_count_match[strTags].append(line['mean_pondere_count_match'])
                tags_to_mean_pondere_volume_match[strTags].append(line['mean_pondere_volume_match'])
                
                # faire les opérations de min et max...
                tags_to_year_max[strTags] = {}
                tags_to_year_min[strTags] = {}
                tags_to_match_count_max[strTags] = {}
                tags_to_match_count_min[strTags] = {}
                tags_to_volume_match_max[strTags] = {}
                tags_to_volume_match_min[strTags] = {}                
                
            else:
                tags_to_occurrences[strTags] = line['somme match count']
                tags_to_year[strTags] = line['nb year']
                tags_to_volume[strTags] = line['somme volume count']
                tags_to_mean_pondere_count_match[strTags] = [line['mean_pondere_count_match']]
                tags_to_mean_pondere_volume_match[strTags] = [line['mean_pondere_volume_match']]
                tags_to_year_max[strTags] = line['year max']
                tags_to_year_min[strTags] = line['year min']
                tags_to_match_count_max[strTags] = line['match count max']
                tags_to_match_count_min[strTags] = line['match count min']
                tags_to_volume_match_max[strTags] = line['volume match max']
                tags_to_volume_match_min[strTags] = line['volume match min']
                
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

header = []
for i in range(nb_ngram):
    header.append('tag '+str(i))
header.extend(['nb year', 'somme match count', 'somme volume count', 'mean pondere count match',\
               'mean pondere volume match', 'year max', 'year min', 'match count max', 'match count min',\
               'volume match max', 'volume match min'])

with open("files/nb-total-occurences-per-tags" +langage+ "-" +nb_ngram+"gram-"+version+".csv", "w", encoding="utf-8-sig", newline='') as f:
    writer = csv.writer(f, delimiter=';', quotechar='"', \
                        quoting=csv.QUOTE_MINIMAL)
    writer.writerow(header)
    
    
    with futures.ThreadPoolExecutor() as executor:
        
        future_to_url = {executor.submit(calculate_nb_total_match_count,\
                                         directory+filename, nb_ngram): filename for filename in filenames}

        for future in futures.as_completed(future_to_url):
            url = future_to_url[future]
            try:
                print('finish :', url)
            except Exception as exc:
                print('%r generated an exception: %s' % (url, exc))
    
    
    writer.writerow()
    

