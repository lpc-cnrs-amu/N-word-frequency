# -*- coding: utf-8 -*-
"""
Created on Thu Jul 25 15:45:43 2019

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
import queue


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


exitFlag = 0
class ThreadOccurrences(threading.Thread):
    def __init__(self, thread_id, name, q, nb_ngram):
        threading.Thread.__init__(self)
        self.thread_id = thread_id
        self.name = name
        self.q = q
        self.nb_ngram = nb_ngram
        
    def run(self):
        calcul_tags_occurrences(self.name, self.q, self.nb_ngram)
        
 
def calcul_tags_occurrences(thread_name, q, nb_ngram=4):    
    global lck, tags_to_occurrences, tags_to_year, tags_to_volume,tags_to_mean_pondere_count_match,\
    tags_to_mean_pondere_volume_match,tags_to_year_max,tags_to_year_min,tags_to_match_count_max,\
    tags_to_match_count_min,tags_to_volume_match_max,tags_to_volume_match_min
    
    while not exitFlag:
        queueLock.acquire()
        if not workQueue.empty():
            filename = q.get()
            print("{} doing: [{}]".format(thread_name, filename))
            queueLock.release()
            with open(filename, "r", encoding="utf-8-sig", newline='') as f:
                reader = csv.DictReader(f, delimiter=';')
                
                for line in reader:
                    tags = []
                    
                    for i in range(1, nb_ngram+1):
                        tags.append(line['tag '+str(i)])
                    
                    strTags = ' '.join(tags)
                    
                    lck.acquire()
                    #print("{} a le verrou".format(thread_name))
                    #print("tags = ", strTags)
                    
                    if strTags in tags_to_occurrences:
                        tags_to_occurrences[strTags] += int(line['somme match count'])
                        tags_to_year[strTags] += int(line['nb year'])
                        tags_to_volume[strTags] += int(line['somme volume count'])
                        tags_to_mean_pondere_count_match[strTags].append(float(line['mean_pondere_count_match']))
                        tags_to_mean_pondere_volume_match[strTags].append(float(line['mean_pondere_volume_match']))
                        
                        if tags_to_year_max[strTags] < int(line['year max']):
                            tags_to_year_max[strTags] = int(line['year max'])
                            
                        if tags_to_year_min[strTags] > int(line['year min']):
                            tags_to_year_min[strTags] = int(line['year min'])
                            
                        if tags_to_match_count_max[strTags] < int(line['match count max']):
                            tags_to_match_count_max[strTags] = int(line['match count max'])
                            
                        if tags_to_match_count_min[strTags] > int(line['match count min']):
                            tags_to_match_count_min[strTags] = int(line['match count min'])
                            
                        if tags_to_volume_match_max[strTags] < int(line['volume match max']):
                            tags_to_volume_match_max[strTags] = int(line['volume match max'])
                        
                        if tags_to_volume_match_min[strTags] > int(line['volume match min']):
                            tags_to_volume_match_min[strTags] = int(line['volume match min'])
                        
                    else:
                        tags_to_occurrences[strTags] = int(line['somme match count'])
                        tags_to_year[strTags] = int(line['nb year'])
                        tags_to_volume[strTags] = int(line['somme volume count'])
                        tags_to_mean_pondere_count_match[strTags] = [float(line['mean_pondere_count_match'])]
                        tags_to_mean_pondere_volume_match[strTags] = [float(line['mean_pondere_volume_match'])]
                        
                        tags_to_year_max[strTags] = int(line['year max'])
                        tags_to_year_min[strTags] = int(line['year min'])
                        tags_to_match_count_max[strTags] = int(line['match count max'])
                        tags_to_match_count_min[strTags] = int(line['match count min'])
                        tags_to_volume_match_max[strTags] = int(line['volume match max'])
                        tags_to_volume_match_min[strTags] = int(line['volume match min'])
                        
                    lck.release()
                    #print("{} relache le verrou".format(thread_name))
            print("{} finish: [{}]".format(thread_name, filename))
        else:
            queueLock.release()    
                        

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
             str(nb_ngram)+'gram-'+version+'-('+strIndexes+')-frequences-pretreat.csv', f)]

header = []
for i in range(nb_ngram):
    header.append('tag '+str(i))
header.extend(['nb year', 'somme match count', 'somme volume count', 'mean pondere count match',\
               'mean pondere volume match', 'year max', 'year min', 'match count max', 'match count min',\
               'volume match max', 'volume match min'])

thread_list = []
nb_threads = 150
for i in range(nb_threads):
    thread_list.append("Thread-"+str(i))

queueLock = threading.Lock()
workQueue = queue.Queue(0)
threads = []
threadID = 1

# Create new threads
for thread_name in thread_list:
    thread = ThreadOccurrences(threadID, thread_name, workQueue, nb_ngram)
    thread.start()
    threads.append(thread)
    threadID += 1
    
# Fill the queue
queueLock.acquire()
for filename in filenames:
    workQueue.put(directory+filename)
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

with open("files/nb-total-occurences-per-tags-" +langage+ "-" +str(nb_ngram)+"gram-"+version+"-2.csv",\
          "w", encoding="utf-8-sig", newline='') as f:
    writer = csv.writer(f, delimiter=';', quotechar='"', \
                        quoting=csv.QUOTE_MINIMAL)
    writer.writerow(header) 
    for tags_grams in tags_to_occurrences:
        row = []
        list_tags_grams = tags_grams.split()
        for tag in list_tags_grams:
            row.append(tag)
        row.extend([tags_to_year[tags_grams], tags_to_occurrences[tags_grams],\
                    tags_to_volume[tags_grams]])
        
        
        mean_pondere_count_match = round(sum(tags_to_mean_pondere_count_match[tags_grams]) / \
                                         len(tags_to_mean_pondere_count_match[tags_grams]), 2)

        mean_pondere_volume_match = round(sum(tags_to_mean_pondere_volume_match[tags_grams]) / \
                                         len(tags_to_mean_pondere_volume_match[tags_grams]), 2)
        
        row.extend([mean_pondere_count_match, mean_pondere_volume_match, tags_to_year_max[tags_grams],\
                    tags_to_year_min[tags_grams], tags_to_match_count_max[tags_grams], \
                    tags_to_match_count_min[tags_grams], \
                    tags_to_volume_match_max[tags_grams], tags_to_volume_match_min[tags_grams]])
        writer.writerow(row)
