# IdentifyShifts
The official implementation of the SIGIR'18 paper "Identify Shifts of Word Semantics through Bayesian Surprise"

----
	The first thing is get the useful information we need from the original dataset.
    
    As described in our paper, we experiment on the below four data set:

        * ACM Abstract: from https://aminer.org/citation, we use ACM-V8 in our experiment;

        * DBLP Abstract: from https://aminer.org/citation, we use DBLP-V10 in our experiment;

        * ACMDL: consists of full text papers of ACM publications;
        
        * Google Books Ngram Dataset: from http://storage.googleapis.com/books/ngrams/books/datasetsv2.html

    In this demo, we take DBLP Abstract as example.

	The form of the dataset is illustrated at https://aminer.org/citation

preTrain
----
    1. Here we just get the abstracts of papers from ./dblp-ref , and put them into ./input/

	And we add "<start>" before each abstract and add "<end>" at the end of the each abstract which marks the begin and the end of each abstract.

	It will print out the total number of papers that have an abstract. The total paper number of DBLP is 753646 + 922383 + 827533 + 44970.

	You can run it by:

	python ./preTrain/dealWith_org_dataset.py -i ./dblp-ref -o ./input/

	2. Then, we use ./preTrain/countDocumentFre.c to count words' document frequency in ../input/

	The results are put into ../document_frequency/

	You can compile it through:
		gcc-4.6 ./preTrain/countDocumentFre.c -o ./preTrain/countDocumentFre -std=c99

	Run it by:
		 ./preTrain/countDocumentFre -i ./input/ -o ./document_frequency/

    The words are in after stemming form.

	3. Next, we apply ./preTrain/accmulateFre.py to accmulate document frequency.

	The results are put into ./accmulate_document_fre/

	Run it by:
		python accumulate_fre.py -i ./document_frequency/ -o ./accmulate_document_fre/

	4. Now, we hope to filter the infrequent words to get the valid word list and avoid noises.

    ./preTrain/getWordList.c counts words' frequency in ./input/ and filters the results by the same conditions with word2vec(the words' length should between 2 and 15, and each words should appears more than 10 in a year).

        The results are put into ./valid/

        You can compile it through:
            gcc-4.6 ./preTrain/getWordList.c -o ./preTrain/getWordList -std=c99

        And run it by:
            ./preTrain/getWordList -i ./input/ -o ./valid/
	
	It will print out the total number of words that statisfying the filtering conditions. 

	5. Now, we hope to count every two words' weight and print it out. For example, the weight between 'apple' and 'banana' is 100, there are two lines in output file 'apple banana 100.00000' and 'banana apple 100.0000'. (This follows LINE's training instructions)

	You can compile raw2graph.c through:
		gcc-4.6 ./preTrain/raw2graph.c -o ./preTrain/acmdl2graph -std=c99

	And run it by:
		./preTrain/acmdl2graph -i ./input/ -v ./fre/ -o ./graph/

    It stores words' freqency in ./fre/, the graph in ./graph/

	6. Then, we use ./preTrain/raw2sentence.c to transfrom the original dataset into each sentence after omitting infrequent words.	
	
	You can compile it through:
		gcc-4.6 ./preTrain/raw2sentence.c -o ./preTrain/raw2sentence -std=c99

	And run it by:
		./preTrain/raw2sentence -i ./input -o ./sentences -v ./valid

trainEmbedding
----
    Now we come to the training embedding section, we present two version of LINE: 

        * line_org.cpp uses randomized initialization
        * line_init.cpp uses last period's embedding as initialization

    To avoid redundant description, we just show how to compile line_init.cpp here and refer readers to grab more details from LINE's original document: https://github.com/tangjianpku/LINE

	1. ./trainEmbedding/line_init.cpp trains all files in ./graph, and puts the results into ./embeddings/first_init/ and ./embeddings/second_init/

	You can compile it through:
		g++-4.6 ./trainEmebdding/line_init.cpp -o ./trainEmbedding/line_init -I ~/.local/include/ -L ~/.local/lib/ -lgsl -lm -lgslcblas -lpthread

	And run it:
		./trainEmbedding/line_init -binary 0 -size 100 -negative 5 -samples 100 -rho 0.025 -threads 16 -train ./graph/ -output ./embeddings/first_init/ -order 1
        ./trainEmbedding/line_init -binary 0 -size 100 -negative 5 -samples 100 -rho 0.025 -threads 16 -train ./graph/ -output ./embeddings/second_init/ -order 2

	2. Open ./trainEmbedding/concatenated.cpp

	Concatenate the first-order priority and second-order priority embeddings, and do normalization.

	You can compile it through:
		g++-4.6 ./trainEmbedding/concatenate.cpp -o ./trainEmbedding/concatenate

	And run it:
		./trainEmbedding/concatenate.o -input1 first_init -input2 second_init -output concatenated_init -binary 0

    3. To decrease the number of words when calculating k-nearest neighbors, it's necessary to filter out infrequent words after getting concatenate embeddings.

    simplify.py scans the words in concatenated_org, aiming at filters out the unstable words whose frequency are very small.

    The result will be put in director ../embeddings/after_filtering_concatenated/

    Run it:
        python simplify.py -i ../embeddings/concatenated_org -hi ../accmulate_document_fre/2016 -th 250 -fr ../document_frequency/ -th1 25 -o ../embeddings/after_filtering_concatenated/

----
    In this section, we will do some preparations before getting the final results.

    1. To filter noises further in getting the results, we use ./getResult/filtering.py to remain the valid words that appreas more 250 times in history, and appears more than 100 times before the previous year and appears more than 50 times in the interval years. 

    Run it:
        python filtering.py -hi ./accmulate_document_fre/2016 -th 250 -fr ./accmulate_document_fre/ -th1 100 -th2 50 -o ./filtering_250_100_50/

    Where   -hi represents the historical document frequency;
            -th represents the threshold; 
            -fr represents the accumulated document frequency's diretory;
            -th1 represents threshold1; 
            -th2 represents threshold2; 
            -o represents output directory.

    2. Then, we hope to calculate the approximate k-nearest neighbors

    Let's open ./k-nearest-graph/main.cpp

    main.cpp and LargeVis.cpp circulates the approximate k-nearest graph of the input embeddings.

    You can find more details on: https://github.com/lferry007/LargeVis

    Here we modify some places in order to fit our needs. The input of our program is a directory that contains many independant years' embeddings and the output is also a directory.

    You can compile it through:
        g++ ./k-nearest-graph/LargeVis.cpp ./k-nearest-graph/main.cpp -o ./k-nearest-graph/LargeVis -lm -pthread -lgsl -lgslcblas -Ofast -march=native -ffast-math -I ~/.local/include/ -L ~/.local/lib/

    Run it:
        ./k-nearest-graph/LargeVis -input ./embeddings/after_filtering_concatenated/ -output ./ann-graph/

    Attention: The nestest node in ANN results has the largest value.

getResult
----
    In this section, we're going to get the final results for competitive algorithms and our proposed models.

    1. initialization.py circulates the Euclidean Distance of two periods' embeddings and lists out the top words that LINE+Init regarded as meaning change words. 

    Run it by:
        python ./getResult/initialization.py -i ./embeddings/after_filtering_concatenated/ -y 1 -d ../filtering_500_100_50/ -p 50 -o LINE+Init.txt

    2. knn+jaccard_dis.py compares two periods' k-nearest graph to get the results of LINE+aNN+Jaccard and LINE+Init+aNN+Jaccard.

    Its input is a knn-graph directory and a filtering directory that filters out the useless words. And it puts the results in a file.

    Run it by:
        python ./getResult/knn+jaccard_dis.py -i ./ann-graph/ -hi ./accmulate_document_fre/2016 -th 250 -d ./filtering_1_250_100_50/ -k 50 -y 1 -p 50 -o knn+jaccard_dis_1_250_100_50.txt

    3. knn+bayesian.py compares two periods' k-nearest graph to get the results by using Bayesian Surprise.

    Its input is also a knn-graph directory and a filtering directory. It puts the results in a file.

    Run it by:
        python ./getResult/knn+bayesian.py -i ./ann-graph/ -hi ../accmulate_document_fre/2016 -th 500 -d ../filtering_1_250_100_50/ -k 50 -y 1 -p 50 -t 1990 -o knn+bayesian_1_250_100_50.txt 

    4. As for alignment-based methods, you're welcome to read more details from: https://github.com/viveksck/langchangetrack

    5. ./getResult/evaluation.py computes the recall and mean average precision.

    Its input is the groundtruth and the result list. Notices: here ./word_lists_200/confusing_words_list1.txt is the synthetic groundtruth

    Run it by:
        python evaluation.py -i LINE+Init.txt -t ./word_lists_200/confusing_words_list1.txt -o result_LINE+Init.txt 
    or:
        python ./getResult/evalution.py -i knn+jaccard_dis_1_250_100_50.txt -s ./word_lists_200/confusing_words_list1.txt -o result_knn+jaccard.txt 
    or:
        python ./getResult/evalution.py -i knn+bayesian_1_250_100_50.txt -s ./word_lists_200/confusing_words_list1.txt -o result_knn+bayesian.txt




