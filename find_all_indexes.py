# -*- coding: utf-8 -*-
"""
Created on Thu Jul 18 10:04:03 2019

@author: Marjorie
"""


id_indexes = []

with open("files/google_url.txt") as f:
    for line in f:
        line = line.strip()
        index = []
        i = len(line)-1
        if line[i] != 'p':
            continue
        
        for j in range(2):
            while line[i] != '.':
                i = i - 1 
            i -= 1
        while line[i] != '-':
            index.append(line[i])
            i = i - 1
        index.reverse()
        strIndex = ''.join(str(e) for e in index)
        #print(strIndex)
        
        if strIndex not in id_indexes:
            id_indexes.append(strIndex)




with open('files/indexes_zip.txt', 'w') as f:
    for elt in id_indexes:
        f.write(elt+"\n")