#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from io import open
from os import path
from argparse import ArgumentParser

def main(args):
    input_dir = args.input_dir
    output_dir = args.output_dir
    dic = {}
    value = []
    n = 0
    for filename in sorted(os.listdir(input_dir)):
        cur_file = path.join(input_dir, filename)

        with open(cur_file, 'r') as f:
	    print filename
            lines = f.readlines()
            for line in lines:
                words = line.split()
                word = str(words[0])
                val = float(words[1])
                if word in dic:
                    value[dic[word]] += val
                else:
                    dic.update({word : n})
                    value.append(val)
                    n += 1

        cur_file = path.join(output_dir, filename)
        with open(cur_file, 'w') as f:
            for word in dic:
                f.write(unicode(word + ' ' + str(value[dic[word]]) + '\n'))
        print filename

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input_dir", dest = "input_dir", help = "The original directory.")
    parser.add_argument("-o", "--output_dir", dest = "output_dir", help = "The output accumulated directory.")
    args = parser.parse_args()
    main(args)
