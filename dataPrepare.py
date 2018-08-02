import subprocess
import os

from os import path

try:
#    print("Downloading raw data.")
#    res = subprocess.check_call("wget https://static.aminer.cn/lab-datasets/citation/dblp.v10.zip", shell = True)
#    print("Raw data downloaded!")
#
#    print("Unzipping the raw data.")
#    res = subprocess.check_call("unzip dblp.v10.zip", shell = True)
#    print("Unzipping finished!")
#
#    if not path.exists("./temp_data"):
#        os.mkdir("./temp_data")
#    if not path.exists("./temp_data/input_raw"):
#        os.mkdir("./temp_data/input_raw")
#    if not path.exists("./temp_data/document_fre")
#        os.mkdir("./temp_data/document_fre")
#    if not path.exists("./temp_data/accumulated_document_fre"):
#        os.mkdir("./temp_data/accumulated_document_fre")
    if not path.exists("./temp_data/wordList"):
        os.mkdir("./temp_data/wordList")
#
#    print("Dividing time line.")
#    res = subprocess.check_call("python ./preTrain/dealWith_org_dataset.py -i ./dblp-ref/dblp-ref-0.json -o ./temp_data/input_raw", shell = True)
#    res = subprocess.check_call("python ./preTrain/dealWith_org_dataset.py -i ./dblp-ref/dblp-ref-1.json -o ./temp_data/input_raw", shell = True)
#    res = subprocess.check_call("python ./preTrain/dealWith_org_dataset.py -i ./dblp-ref/dblp-ref-2.json -o ./temp_data/input_raw", shell = True)
#    res = subprocess.check_call("python ./preTrain/dealWith_org_dataset.py -i ./dblp-ref/dblp-ref-3.json -o ./temp_data/input_raw", shell = True)
#    print("Time line cutted.")

#    print("Counting words' document frequency in raw data.")
#    res = subprocess.check_call("gcc ./preTrain/countDocumentFre.c -o ./preTrain/countDocumentFre -std=c99", shell = True)
#    res = subprocess.check_call("./preTrain/countDocumentFre -i ./temp_data/input_raw -o ./temp_data/document_fre")
#    print("Finished!")

#    print("Accumulating document frequencies. (This step serves for some filterings later)")
#    res = subprocess.check_call("python ./preTrain/accumulateFre.py -i ./temp_data/document_fre -o ./temp_data/accumulated_document_fre", shell = True)
#    print("Finsihed!")
    
    print("Adopting the same filter condition with word2vec. Each word in the word lists whose length is between 2 and 15, and each word appears more than 10 times in one certain year.")
    res = subprocess.check_call("gcc ./preTrain/getWordList.c -o ./preTrain/getWordList -std=c99", shell = True)
    res = subprocess.check_call("./preTrain/getWordList -i ./temp_data/input_raw -o ./temp_data/wordList", shell = True)
    print("Done!")

except subprocess.CalledProcessError, exc:
    print('cmd:', exc.cmd)
