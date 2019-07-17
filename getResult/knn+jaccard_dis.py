#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np

from io import open
from os import path
from operator import itemgetter
from argparse import ArgumentParser
from scipy.sparse import dok_matrix

class datatype(object):
    def __init__(self, n = 0, mat = dok_matrix((0, 0), dtype = np.float64), knn = []):
        self.n = n
        self.mat = mat
        self.knn = knn

    def get_n(self):
        return self.n

    def get_mat(self):
        return self.mat

    def get_knn(self):
        return self.knn

    def store_n(self, n):
        self.n = n

    def store_mat(self, mat):
        self.mat = mat

    def store_knn(self, knn):
        self.knn = knn

def load(filename, data, wordlist, dic, n, k, fil, judge, map_dic, ak):
    temp = [[] for x in xrange(n)]
    with open(filename, 'r') as f:
        lines = f.readlines()
        for line in lines:
            word1 = line.split()[0]
            word2 = line.split()[1]
            value = float(line.split()[2])

            if fil.has_key(word1) and wordlist.has_key(word1) and wordlist.has_key(word2):
                temp[wordlist[word1]].append((wordlist[word2], value))

    mat = dok_matrix((n, n), dtype = np.float64)
    knn = [[] for x in xrange(n)]

    for word in xrange(len(temp)):
        wlist = temp[word]
        if len(wlist) >= k:
            wlist = sorted(wlist, key = itemgetter(1), reverse = ak)
            for i in xrange(k):
                knn[word].append(wlist[i][0])
                mat[word, wlist[i][0]] += wlist[i][1]
                if judge:
                    map_dic.update({(word, wlist[i][0]):wlist[i][1]})
    
    data.store_knn(knn)
    data.store_n(n)
    data.store_mat(mat)

def deal(pre_data, cur_data, presentation, output_file, wordlist, dic, n, cur_year, k, fil, map_dic, year, ak):
    val_a = np.zeros(n)
    val_b = np.zeros(n)
    val_intersection = np.zeros(n)
    mat1 = pre_data.get_mat()
    mat2 = cur_data.get_mat()
    knn1 = pre_data.get_knn()
    knn2 = cur_data.get_knn()
    res = []

    with open(output_file, 'a') as f:
        tempdic = {}
        for i, j in mat1.iterkeys():
            val_a[i] += 1
            if map_dic.has_key((i, j)):
                val_intersection[i] += 1
        for i, j in mat2.iterkeys():
            val_b[i] += 1
        for i in xrange(n):
            if fil.has_key(dic[i]) and val_a[i] == k and val_b[i] == k:
                res.append([dic[i], 1 - val_intersection[i] / (val_a[i] + val_b[i] - val_intersection[i]), 0])
            else:
                res.append([dic[i], 0, 0])

        for i in xrange(n):
            if res[i][1] != 0:
                for j in knn1[i]:
                    res[i][2] += 1 - res[j][1]
                for j in knn2[i]:
                    res[i][2] += 1 - res[j][1]
                    if res[j][1] == 0 and not fil.has_key(dic[i]):
                        print dic[j]

        if n < presentation:
            presentation = n
        res = sorted(res, key = itemgetter(1, 2), reverse = True)
        f.write(unicode("The results between {} and {} are listed below:\n".format(int(cur_year[:4]) - year, cur_year)))
        for i in xrange(presentation):
            if res[i][1] == 0:
                break
            f.write(unicode('\t\t' + res[i][0]))
#            f.write(unicode(' : %.4f %.4f'%(res[i][1], res[i][2])))
            if (i + 1) % 4 == 0:
                f.write(unicode('\n'))
        i = presentation
        while i < n and res[i][1] == res[presentation - 1][1] and res[i][2] == res[presentation - 1][2]:
            if res[i][1] == 0:
                break
            f.write(unicode('\t\t' + res[i][0]))
 #           f.write(unicode(' : %.4f'%(res[i][1], res[i][2])))
            if (i + 1) % 4 == 0:
                f.write(unicode('\n'))
            i += 1

        f.write(unicode('\n'))
            
def build_dic(history, threshold, dic, wordlist):
    n = 0
    with open(history, "r") as f:
        lines = f.readlines()
        for line in lines:
            word = line.split()[0]
            number = float(line.split()[1])
            if number >= threshold and word not in wordlist:
                wordlist.update({word:n})
                dic.append(word)
                n += 1
    return n

def main(args):
    input_dir = args.input_dir
    output_file = args.output_file
    history = args.history
    threshold = args.threshold
    dictionary = args.dictionary
    year = args.year
    presentation = args.presentation
    k = args.k
    ak = args.ak
    time = args.time
    
    pre_data = datatype()
    cur_data = datatype()
    if path.isfile(output_file):
        os.remove(output_file)

    wordlist = {}
    dic = []
    n = build_dic(history, threshold, dic, wordlist)

    print n
    for filename in sorted(os.listdir(input_dir)):
        pre_file = path.join(input_dir, str(int(filename[:4]) - year))
        cur_file = path.join(input_dir, filename)
        if path.exists(pre_file) and path.exists(cur_file) and int(filename[:4]) > time:
            map_dic = {}
            print pre_file + " -> " + filename
            fil = {}
            filtering = path.join(dictionary, filename[:4])
            with open(filtering, 'r') as f:
                lines = f.readlines()
                for line in lines:
                    fil.update({str(line.split()[0]) : 0})

            load(pre_file, pre_data, wordlist, dic, n, k, fil, False, map_dic, ak)
            n = pre_data.get_n()

            load(cur_file, cur_data, wordlist, dic, n, k, fil, True, map_dic, ak)
            n = cur_data.get_n()

            deal(pre_data, cur_data, presentation, output_file, wordlist, dic, n, filename, k, fil, map_dic, year, ak)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input_dir", dest = "input_dir", help = "Input directory.")
    parser.add_argument("-o", "--output_file", dest = "output_file", help = "Output file.")
    parser.add_argument("-hi", "--history", dest = "history", help = "The accumulated historical document frequency.")
    parser.add_argument("-th", "--threshold", dest = "threshold", type = int, default = 250, help = "Use threshold to filter out the unfrequency words in kNN.")
    parser.add_argument("-d", "--dictionary", dest = "dictionary", help = "The words after filtering.")
    parser.add_argument("-k", "--k_value", dest = "k", type = int, default = 50, help = "The number of nearest nodes.")
    parser.add_argument("-y", "--year", type = int, default = 5, dest = "year", help = "The intervel of the comparison.")
    parser.add_argument("-t", "--time", type = int, default = 1990, dest = "time", help = "The start time of exchanging word lists.")
    parser.add_argument("-p", "--presentation", dest = "presentation", type = int, default = 50, help = "The windows' size.")
    parser.add_argument("-ak", "--annorknn", dest = "ak", type = eval, default = True, help = "If the input is sorted by knn, the value should be False; otherwise the value should be True.")
    args = parser.parse_args()
    main(args)

