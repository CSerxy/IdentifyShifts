import os
import numpy as np
from sets import Set

from io import open
from os import path
from argparse import ArgumentParser

def read_change_words(change_word_file, wordlist):
    with open(change_word_file, "r") as f:
        lines = f.readlines()
        for line in lines:
            year = int(line.split()[0])
            word1 = line.split()[1]
            word2 = line.split()[2]
            if year in wordlist:
                wordlist[year].add(word1)
                wordlist[year].add(word2)
            else:
                wordlist[year] = Set([word1, word2])
def is_int(s):
    try:
        int(s)
        return True
    except ValueError:
        return False

def read_results(input_file, result_list):
    with open(input_file, "r") as f:
        lines = f.readlines()
        for line in lines:
            if ":" in line:
                for i in line.split():
                    if is_int(i):
                        cur_year = int(i)
                result_list[cur_year] = []
            else:
                for i in line.split():
                    result_list[cur_year].append(i)
            
def get_recall(wordlist, result_list):
    rr = 0.0
    nr = 0.0
    for year, words in wordlist.iteritems():
        for word in words:
            if word in result_list[year]:
                rr += 1
            else:
                nr += 1
    return rr / (rr + nr)

def get_map(wordlist, result_list):
    mean_average_precision = 0.0
    number_of_year = 0
    for year, results in result_list.iteritems():
        if wordlist.has_key(year):
            number_of_year += 1
            rr = 0.0
            ir = 0.0
            precision = 0.0
            total = len(wordlist[year])
            for word in results:
                if word in wordlist[year]:
                    rr += 1
                    precision += rr / (rr + ir)
                else:
                    ir += 1
            mean_average_precision += precision / total
    return mean_average_precision / number_of_year

def main(args):
    input_file = args.input_file
    change_word_file = args.change_word_file
    
    wordlist = {}
    read_change_words(change_word_file, wordlist)

    result_list = {}
    read_results(input_file, result_list)

    recall = get_recall(wordlist, result_list)

    mean_average_precision = get_map(wordlist, result_list)

    print recall
    print mean_average_precision

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input_file", dest = "input_file", help = "The input file name.")
    parser.add_argument("-s", "--switch_list", dest = "change_word_file", default = "../word_lists_200/confusing_words_list1.txt", help = "The switched words' file name.")
    args = parser.parse_args()
    main(args)
