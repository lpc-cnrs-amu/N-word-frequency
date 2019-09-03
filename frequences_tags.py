# -*- coding: utf-8 -*-
"""
Created on Mon Aug 26 11:07:21 2019

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

langage = "fre"
nb_ngram = "4"
version = "20120701"

df = pd.read_csv("files/nb-total-occurences-per-tags-" +langage+ "-" +\
                 nb_ngram+"gram-"+version+".csv", sep=';', encoding='utf-8-sig')

# all the occurrences of each tag-grams
total_match_count = df['somme match count'].sum()
# total number of volumes
total_volume_count = df['somme volume count'].sum()

df['freq match count'] = df['somme match count'] / total_match_count
df['freq volume count'] = df['somme volume count'] / total_volume_count


df.to_csv("files/nb-total-frequences-per-tags-" +langage+ "-" +\
          nb_ngram+"gram-"+version+".csv", index=False, na_rep='NaN', encoding='utf-8-sig', sep=";")