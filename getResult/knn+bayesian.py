#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

from io import open
from os import path
from operator import itemgetter
from argparse import ArgumentParser
from scipy.sparse import dok_matrix
from scipy import special as sp


def build_dic(history, threshold, wordlist, dic):
    n = 0
    with open(history, "r") as f:
        lines = f.readlines()
        for line in lines:
            word = line.split()[0]
            number = float(line.split()[1])
            if number > threshold and word not in wordlist:
                wordlist.update({word:n})
                dic.append(word)
                n += 1
    return n

def pre_training(n, k, year, input_dir, training, wordlist, dic, alpha, check, num_of_each_line, each_line, ak):
#    count = 0
    for filename in sorted(os.listdir(input_dir)):
       if int(filename) <= training:
            temp = [[] for i in xrange(n)]
            temp_dic = []
            cur_file = path.join(input_dir, filename)
            print cur_file

            if path.exists(cur_file):
                with open(cur_file, "r") as f:
                    lines = f.readlines()
                    for line in lines:
                        word1 = line.split()[0]
                        word2 = line.split()[1]
                        value = float(line.split()[2])
                        if word1 in wordlist and word2 in wordlist:
                            temp[wordlist[word1]].append((wordlist[word2], value))

                for word in xrange(len(temp)):
                    if len(temp[word]) >= k:
                        wlist = temp[word]
                        
                        wlist = sorted(wlist, key = itemgetter(1), reverse = ak)
                        for i in xrange(k):
                            if (word, wlist[i][0]) in check:
#                                if count == 0:
#                                    alpha[cur_year][word][check[(word, wlist[i][0])]] += 1
#                                else:
                                alpha[word][check[(word, wlist[i][0])]] = alpha[word][check[(word, wlist[i][0])]] + 1
                            else:
                                each_line[word].append(wlist[i][0])
                                alpha[word].append(2)
#Here we use Add-one smoothing, every words appears at first time will add one as default.
                                check.update({(word, wlist[i][0]):num_of_each_line[word]})
                                num_of_each_line[word] += 1
#            count += 1

## This function is used to circulate the value of
## log(gamma(q) / gamma(p))
## As you know, the maximum value that Python can handle
## of gamma function is gamma(171)
## If the p or q is larger than 171, we have to approximate
## the result
def log_division_of_two_gamma_function(q, p):
    if p <= 171 and q <= 171:
        result = np.log(sp.gamma(q) / sp.gamma(p))
    elif float(p).is_integer() and float(q).is_integer():
        result = 0
        if q < p:
            for i in xrange(int(q) - 1, int(p) - 1):
                result -= np.log(i)
        else:
            for i in xrange(int(p) - 1, int(q) - 1):
                result += np.log(i)
    else:
        result = (q - p) * (-1) + (p - 0.5) * np.log((q - 1) / (p - 1)) + (q - p) * np.log(q - 1)

    return result

def KL_divergence(qt, pt):
    result = 0
    if len(qt) != len(pt):
        print "The length of", qt, "and", pt, "is different!"
        exit(0)

    sum_qt = 0
    sum_pt = 0
    for i in qt:
        sum_qt += i
    for i in pt:
        sum_pt += i 

    result += log_division_of_two_gamma_function(sum_qt, sum_pt)

    diqt = sp.digamma(sum_qt)
    for i in xrange(len(qt)):
        result += log_division_of_two_gamma_function(pt[i], qt[i]) + (qt[i] - pt[i]) * (sp.digamma(qt[i]) - diqt)

    return result

def train(n, k, year, presentation, input_dir, output_file, training, wordlist, dic, alpha, check, num_of_each_line, each_line, ak, dictionary):
    for filename in sorted(os.listdir(input_dir)):
        temp = [[] for i in xrange(n)]
        temp_dic = []
        res = []
        if int(filename) > training:
            cur_file = path.join(input_dir, filename)
            print cur_file

            if path.exists(cur_file):
                with open(cur_file, "r") as f:
                    lines = f.readlines()
                    for line in lines:
                        word1 = line.split()[0]
                        word2 = line.split()[1]
                        value = float(line.split()[2])
                        if word1 in wordlist and word2 in wordlist:
                            temp[wordlist[word1]].append((wordlist[word2], value))
                fil = {}
                dic_doc = path.join(dictionary, filename)
                with open(dic_doc, 'r') as f:
                    lines = f.readlines()
                    for line in lines:
                        fil.update({str(line.split()[0]) : 0})

                for word in xrange(len(temp)):
                    if len(temp[word]) >= k:
                        temp_result = 0
                        wlist = temp[word]
                        wlist = sorted(wlist, key = itemgetter(1), reverse = ak)
                        res_ = {}
                        new_alpha = []
                        add_ = False
                        for i in xrange(k):
                            res_.update({wlist[i][0]:1})
                            if (word, wlist[i][0]) not in check:
                                each_line[word].append(wlist[i][0])
                                alpha[word].append(1)
                                check.update({(word, wlist[i][0]):num_of_each_line[word]})
                                num_of_each_line[word] += 1
    
                        for s in xrange(num_of_each_line[word]):
                            if each_line[word][s] in res_:
                                new_alpha.append(alpha[word][s] + 1)
                            else:
                                new_alpha.append(alpha[word][s])
    
                        temp_result = KL_divergence(new_alpha, alpha[word])

                        alpha[word] = new_alpha

                        res.append((dic[word], temp_result))

                nn = len(res)
                with open(output_file, 'a') as f:
                    if nn < presentation:
                        presentation = nn
                    count = 0
                    i = 0
                    res = sorted(res, key = itemgetter(1), reverse = True)
                    f.write(unicode("The results between {} and {} are listed below:\n").format(int(filename) - year, filename))

                    while count < presentation and i < nn:
                        if res[i][0] in fil:
                            f.write(unicode('\t\t' + res[i][0]))
                            if (count + 1) % 4 == 0:
                                f.write(unicode('\n'))
                            last_i = i
                            count += 1
                        i += 1
                    while i < nn and res[i][1] == res[last_i][1]:
                        if res[i][0] in fil:
                            f.write(unicode('\t\t' + res[i][0]))
                            if (count + 1) % 4 == 0:
                                f.write(unicode('\n'))
                            count += 1
                        i += 1

                    f.write(unicode('\n'))

def main(args):
    input_dir = args.input_dir
    output_file = args.output_file
    dictionary = args.dictionary
    presentation = args.presentation
    k = args.k
    training = args.training
    history = args.history
    threshold = args.threshold
    year = args.year
    ak = args.ak

    dic = []
    wordlist = {}
    n = build_dic(history, threshold, wordlist, dic)

    check = {}
    print n
    num_of_each_line = np.zeros(n, dtype = np.int)

    each_line = [[] for i in xrange(n)]
    alpha = [[] for i in xrange(n)]
#    alpha = dok_matrix((n, n), dtype = np.float64)
    pre_training(n, k, year, input_dir, training, wordlist, dic, alpha, check, num_of_each_line, each_line, ak)

    train(n, k, year, presentation, input_dir, output_file, training, wordlist, dic, alpha, check, num_of_each_line, each_line, ak, dictionary)
    



if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input_dir", dest = "input_dir", help = "Input directory.")
    parser.add_argument("-o", "--output_file", dest = "output_file", help = "Output file.")
    parser.add_argument("-d", "--dictionary", dest = "dictionary", help = "The words after filtering.")
    parser.add_argument("-k", "--k_value", dest = "k", type = int, default = 50, help = "The number of nearest nodes.")
    parser.add_argument("-y", "--year", type = int, default = 5, dest = "year", help = "The intervel of the comparison.")
    parser.add_argument("-p", "--presentation", dest = "presentation", type = int, default = 50, help = "The windows' size.")
    parser.add_argument("-t", "--traning", dest = "training", type = int, default = 1985, help = "The training stop years.")
    parser.add_argument("-hi", "--historical", dest = "history", help = "The historical word list.")
    parser.add_argument("-th", "--threshold", dest = "threshold", type = int, default = 250, help = "The historical threshold.")
    parser.add_argument("-ak", "--annorknn", dest = "ak", type = eval, default = True, help = "If the input is sorted by knn, the value should be False; otherwise, the value should be true.")
    args = parser.parse_args()
    main(args)
