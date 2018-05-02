import os
import io

from os import path
from argparse import ArgumentParser

def build_dic(history, threshold, wordlist):
    n = 0
    with open(history, "r") as f:
        lines = f.readlines()
        for line in lines:
            word = line.split()[0]
            number = float(line.split()[1])
            if number > threshold and word not in wordlist:
                wordlist.update({word:n})
                n += 1

def output(out_file, resultlist):
    with open(out_file, "w") as f:
        for word in resultlist:
            f.write(word)
            f.write('\n')

def filtering(wordlist, input_dir, threshold1, threshold2, output_dir, year):
    st = int(sorted(os.listdir(input_dir))[0])
    for filename in sorted(os.listdir(input_dir)):
        if int(filename) >= st + year:
            cur_file = path.join(input_dir, filename)
            pre_file = path.join(input_dir, str(int(filename) - year))
            
            templist = {}
            with open(pre_file, "r") as f:
                lines = f.readlines()
                for line in lines:
                    word = line.split()[0]
                    number = float(line.split()[1])
                    if number >= threshold1 and word in wordlist and word not in templist:
                        templist.update({word:number})

            resultlist = {}
            with open(cur_file, "r") as f:
                lines = f.readlines()
                for line in lines:
                    word = line.split()[0]
                    number = float(line.split()[1])
                    if word in wordlist and word in templist and word not in resultlist and number - templist[word] >= threshold2:
                        resultlist.update({word:number})
            
            out_file = path.join(output_dir, filename)
            output(out_file, resultlist)


def main(args):
    history = args.history
    threshold = args.threshold
    input_dir = args.frequency
    threshold1 = args.threshold1
    threshold2 = args.threshold2
    output_dir = args.output_file
    year = args.year

    wordlist = {}
    build_dic(history, threshold, wordlist)
    
    filtering(wordlist, input_dir, threshold1, threshold2, output_dir, year)

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-hi", "--history", dest = "history", help = "Filter out words appearing no more than threshold times.")
    parser.add_argument("-th", "--threshold", dest = "threshold", type = int, default = 500, help = "The historical threshold.")

    parser.add_argument("-fr", "--frequency", dest = "frequency", help = "The accumlated document frequency directory.")
    parser.add_argument("-th1", "--threshold1", dest = "threshold1", type = int, default = 100, help = "A word should appear more than threshold1 times before the previous year.")
    parser.add_argument("-th2", "--threshold2", dest = "threshold2", type = int, default = 50, help = "A word should appear more than threshold2 times in current training year.")
    parser.add_argument("-o", "--output", dest = "output_file", help = "The words directory that stores the words after filtering.")
    parser.add_argument("-y", "--year", dest = "year", type = int, default = 1, help = "The time interval.")
    args = parser.parse_args()
    main(args)
