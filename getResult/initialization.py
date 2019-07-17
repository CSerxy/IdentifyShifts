#!/usr/bin/env python
# _*_ coding: utf-8 _*_

import os
import math
import numpy as np

from io import open
from os import path
from operator import itemgetter
from argparse import ArgumentParser

def build_dic(filtering, dic):
    with open(dic, "r") as f:
        lines = f.readlines()
        for line in lines:
            filtering[line.split()[0]] = 0

def load(dic, cur_file, filtering):
    with open(cur_file, "r") as f:
        line = f.readline()
        n = int(line.split()[0])
        dim = int(line.split()[1])

        lines = f.readlines()
        for line in lines:
            word = line.split()[0]

            if word in filtering:
                temp_list = []
                for num in line.split()[1:]:
                    temp_list.append(float(num))
                dic[word] = temp_list

def get_euclidean_dis(a, b, word, pre_year, cur_year):
    if len(a) != len(b):
        print "The embedding length of {} between {} and {} is different.\n".format(word, pre_year, cur_year)
        exit(0)
    dim = len(a)
    result = 0
    for i in xrange(0, dim, 1):
        result += (a[i] - b[i])**2
    result = math.sqrt(result)
    return result

def main(args):
    input_dir = args.input_dir
    year = args.year
    dictionary_dir = args.dictionary_dir
    presentation = args.presentation
    output_file = args.output_file
    training = args.training

    with open(output_file, "w") as outf:
        for filename in sorted(os.listdir(input_dir)):
            if int(filename) > training:
                pre_file = path.join(input_dir, str(int(filename) - year))
                cur_file = path.join(input_dir, filename)
                dic = path.join(dictionary_dir, filename)
                if path.exists(pre_file) and path.exists(cur_file):
                    print str(int(filename) - year) + '-->' + filename
                    filtering = {}
                    build_dic(filtering, dic)
        
                    pre_dic = {}
                    load(pre_dic, pre_file, filtering)
        
                    cur_dic = {}
                    load(cur_dic, cur_file, filtering)
        
                    res = []
                    for word, temp_list in cur_dic.iteritems():
                        if word in pre_dic:
                            res.append((word, get_euclidean_dis(temp_list, pre_dic[word], word, str(int(filename) - year), filename)))
                    
                    res = sorted(res, key = itemgetter(1), reverse = True)
                    
                    if len(res) < presentation:
                        p = len(res)
                    else:
                        p = presentation
    
                    outf.write(unicode("The results between {} and {} are listed below:\n").format(int(filename) - year, filename))
                    for i in xrange(p):
                        outf.write(unicode('\t\t' + res[i][0]))
                        if (i + 1) % 4 == 0:
                            outf.write(unicode('\n'))
                    i = p
                    while (i < len(res) and res[i][1] == res[p - 1][1]):
                        outf.write(unicode('\t\t' + res[i][0]))
                        if (i + 1) % 4 == 0:
                            outf.write(unicode('\n'))
                        i = i + 1
                    outf.write(unicode('\n'))

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input_dir", dest = "input_dir", help = "Embedding directory.")
    parser.add_argument("-y", "--year", type = int, default = 5, dest = "year", help = "The interval of the comparison.")
    parser.add_argument("-d", "--dictionary", dest = "dictionary_dir", help = "The words after filtering.")
    parser.add_argument("-p", "--present", dest = "presentation", type = int, default = 50, help = "The number of words that will be presented.")
    parser.add_argument("-o", "--output_file", dest = "output_file", help = "The output file.")
    parser.add_argument("-t", "--traning", dest = "training", type = int, default = 1985, help = "The training stop years.")
    args = parser.parse_args()
    main(args)
