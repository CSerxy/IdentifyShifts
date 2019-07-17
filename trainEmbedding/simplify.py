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
    print "There are {} words whose document frequency are larger than {} times.\n".format(n, threshold)

def build_frequency_dic(document, threshold, frelist, wordlist):
    n = 0
    with open(document, "r") as f:
        lines = f.readlines()
        for line in lines:
            word = line.split()[0]
            number = float(line.split()[1])
            if number > threshold and word in wordlist and word not in frelist:
                frelist.update({word:n})
                n += 1

def main(args):
    history = args.history
    threshold = args.threshold
    frequency = args.frequency
    threshold1 = args.threshold1
    input_dir = args.input_dir
    output_dir = args.output_dir

    wordlist = {}
    build_dic(history, threshold, wordlist)
    
    for filename in sorted(os.listdir(input_dir)):
        print filename
        cur_file = path.join(input_dir, filename)
        out_file = path.join(output_dir, filename)
        frelist = {}
        fre_file = path.join(frequency, filename)
        build_frequency_dic(fre_file, threshold1, frelist, wordlist)

        n = 0
        with open(cur_file, 'r') as f:
            line = f.readline()
            dimension = int(line.split()[1])
            lines = f.readlines()
            for line in lines:
                word = line.split()[0]
                if word in frelist:
                    n += 1

        with open(out_file, 'w') as o:
            o.write("{} {}\n".format(n, dimension))
            for line in lines:
                word = line.split()[0]
                if word in frelist:
                    o.write(line)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input", dest = "input_dir", help = "The original word embeddings.")
    parser.add_argument("-hi", "--history", dest = "history", help = "Filter out words appearing no more than threshold times.")
    parser.add_argument("-th", "--threshold", dest = "threshold", type = int, default = 250, help = "The historical threshold.")
    parser.add_argument("-fr", "--frequency", dest = "frequency", help = "The document frequency directory.")
    parser.add_argument("-th1", "--threshold1", dest = "threshold1", type = int, default = 25, help = "The each training period threshold.")
    parser.add_argument("-o", "--output", dest = "output_dir", help = "After filtering, the results will put into output_dir.")
    args = parser.parse_args()
    main(args)
