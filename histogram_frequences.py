# -*- coding: utf-8 -*-
"""
Created on Wed Jul 24 14:44:05 2019

@author: Marjorie
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import re


langage = 'fre'
version = '20120701'
indexes = []

#take all the indexes
if not indexes or (indexes and 'all' in indexes):
    indexes = []
    with open('files/indexes.txt', 'r') as f:
        for line in f:
            indexes.append(line.strip())
        
# append a | between each indexes for the regex 
strIndexes = '|'.join(map(str, indexes))

directory = os.getcwd()+'/results/freqNword/'
filenames = [f for f in os.listdir(directory) if re.match(r''+langage+'-all-1gram-'+version+'-('+strIndexes+').csv', f)]

if filenames:
    print("find : \n",filenames)

    """ Def all the Dataset """
    df = pd.DataFrame()
    for filename in filenames:
        print("read ", filename)
        tmp = pd.read_csv(directory+filename, sep=';')
        print("\tnb lines : ", tmp.shape)
        df = pd.concat([df, tmp])
        
    print("nb of lines : ", df.shape)
    
    df = df.astype({'somme match count': 'float'})
    
    df['somme match count log10'] = np.log10(df['somme match count'])
    
    df_filtered = df.loc[df['somme match count log10'] >= 2]
    
    df_filtered['somme match count log10'].plot.hist(bins=500, alpha=0.5)
    plt.show()
    plt.clf()
else:
    print("URL not found with :\n", indexes)