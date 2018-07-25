import os
import io

from os import path
from argparse import ArgumentParser

def main(args):
    input_file = args.input_file
    output_dir = args.output_dir
    count = 0
#    count2 = 0
    if not path.exists(output_dir):
        os.mkdir(output_dir)

    with open(input_file, "rb") as f:
        lines = f.readlines()
        for line in lines:
#            count2 += 1
#            if count2 % 10000 == 0:
#                print count2
            
            if line != "\n" and line.startswith("{\"abstract\": \""):
                count += 1
                temp = line.split("{\"abstract\": \"")[1]
                temp = temp.split("\", \"authors\":")[0]
                year = line.split("\"year\": ")[1].split(", \"id\": ")[0]
                new_file = path.join(output_dir, year)
                w = open(new_file, "a")
                w.write("<start>\t")
                w.write(temp)
                w.write("\n<end>\n")
                w.close()

    print "The number of paper in " + input_file + " is: " + str(count)
if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-i", "--input_file", dest = "input_file", help = "Input file.")
    parser.add_argument("-o", "--output_dir", dest = "output_dir", help = "Output directory.")
    args = parser.parse_args()
    main(args)
