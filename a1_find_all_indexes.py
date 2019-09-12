#!/usr/bin
# -*- coding: utf-8 -*-
"""
Created on Thu Jul 18 10:04:03 2019

@author: Marjorie
"""
import sys
import os

if len(sys.argv) != 3 :
    print("Usage : {} {} {}".format(sys.argv[0], "file_with_URLs", "{gz or zip}"))
    sys.exit

if not os.path.exists(os.path.join("files", "freqNword")):
    os.makedirs(os.path.join("files", "freqNword"))

if sys.argv[2] == "gz":
    end = 'z'
    output_filename = os.path.join("files", "indexes_gz.txt")
elif sys.argv[2] == "zip":
    end = 'p'
    output_filename = os.path.join("files", "indexes_zip.txt")
else:
    print("Last argument must be 'gz' or 'zip'. Example:\n\
          {} google_urls.txt gz".format(sys.argv[0]))
    sys.exit   

id_indexes = []
with open(sys.argv[1], "r") as f:
    for line in f:
        line = line.strip()
        index = []
        i = len(line)-1
        if line[i] != end:
            continue
        
        if end == 'p':
            for j in range(2):
                while line[i] != '.':
                    i -= 1 
                i -= 1
        else:
            while line[i] != '.':
                i -= 1 
            i -= 1
            
        while line[i] != '-':
            index.append(line[i])
            i = i - 1
        index.reverse()
        strIndex = ''.join(str(e) for e in index)
        
        if strIndex not in id_indexes:
            id_indexes.append(strIndex)

with open(output_filename, 'w') as f:
    for elt in id_indexes:
        f.write(elt+"\n")
print("Write in {} all the indexes.".format(output_filename))