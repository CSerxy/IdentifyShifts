	1. The first thing is get the useful information we need from the original dataset.

	The form of the dataset is illustrated at https://aminer.org/citation

	Here we just get the abstracts of papers from ../dblp-ref , and put them into ../input_org/*

	And we add "<start>" before each abstract and add "<end>" at the end of the each abstract which marks the begin and the end of each abstract.

	It will print out the total number of papers that have an abstract. The total paper number of DBLP is 753646 + 922383 + 827533 + 44970.

	You can run it by:

	python dealWith_org_dataset.py -i ../dblp-ref -o ../input_year_org

	2. Open ../codes/firstdeal.c

	firstdeal.c counts words' document frequency in ../input

	The results are put into ../document_frequency

	You can compile it through:
		gcc-4.6 firstdeal.c -o  firstdeal.o -std=c99

	Run it by:
		 ./firstdeal.o -i ../input_year_org/ -o ../document_fre_org

#   The words are in after stemming form.

	3. Open ../codes/accmulate_fre.py
	
	accmulate_fre.py counts the accmulate frequency

	The results are put into ../accmulate_document_fre

	Run it by:
		python accumulate_fre.py -i ../document_fre_org -o ../accmulate_document_fre_org

	4. Enter ../codes, open file acmdl_second_frequency.c

        acmdl_second_frequency.c counts words' frequency in ../input and filters the results by the same conditions with word2vec(the words' length should between 2 and 15, and each words should appears more than 10 in a year).

        The results are put into ../frequency

        You can compile it through:
                gcc-4.6 acmdl_second_frequency.c -o acmdl_second_frequency.o -std=c99

        And run it by:
                ./acmdl_second_frequency.o -i input -o frequency
	
	It will print out the total number of words that statisfying the filtering conditions. In ACM, it totally appears 212713166 words.

	6. Open ../codes/acmdl2graph.c

	acmdl2graph.c counts every two words' weight and print it out.

	 For example, the weight between 'apple' and 'banana' is 100, there are two lines in output file 'apple banana 100.00000' and 'banana apple 100.0000'. (Attension: this place may has bugs)

	You can compile it through:
		gcc-4.6 acmdl2graph.c -o acmdl2graph.o -std=c99

	And run it by:
		./acmdl2graph.o -i ../input_year_org/ -v ../fre_org/ -o ../graph_org/

	7. Open ../codes/acmdl2sentence.c

	acmdl2sentence.c transfroms the original dataset into each sentence after omitting some words that not satisfies our requirement.	
	
	You can compile it through:
		gcc-4.6 acmdl2sentence.c -o acmdl2sentence.o -std=c99

	And run it by:
		./acmdl2sentence.o -i input -o sentences -v frequency

	8. Open ../codes/test_dataset.py

	test_dataset.py tests our dataset using PMI matrix. It circulates out each periods' PMI matrix, and minus the two periods' matrix to see whether this dataset is good enough for us to use.

	Before you run it, you should enter  ../../testdataset to active virtual environment for python.

	So firstly:
		source ../../testdataset/bin/activate

	Then:
		python test_dataset.py -i sentences -o test_dataset -r test_dataset_result -y 5 -w 5
	
	9. Open ../codes/line.cpp

	This is what we want to train.
	
	It trains all the file in folder graph, and puts the results into ../embeddings/second_org/

	You can compile it through:
		g++-4.6 line.cpp -o line.o -I /storage6/foreseer/users/zhuofeng/.local/include/ -L /storage6/foreseer/users/zhuofeng/.local/lib/ -lgsl -lm -lgslcblas -lpthread

	And run it:
		./line.o -binary 0 -size 100 -negative 5 -samples 100 -rho 0.025 -threads 16 -train graph -output first_org -order 1

	10. Open ../codes/concatenated.cpp

	Concatenate the first-order priority and second-order priority embeddings, and do normalization.

	You can compile it through:
		g++-4.6 concatenate.cpp -o concatenate.o

	And run it:
		./concatenate.o -input1 first_org -input2 second_org -output concatenated_org -binary 0

	11. (a) Open ../codes/get_result.cpp

#   Here we get the LINE+Init result

	get_result.cpp uses many filtering conditions such as document frequency, WordNet, total frequency and so on to get what we want.

	You can compile it through:
		g++-4.6 get_result.cpp -o get_result.o

	And run it by:
		./get_result.o -input concatenated_org -dictionary frequency -period 5 -total frequency.txt -threshold 250 -frequency accmulate_document_fre -threshold1 100 -threshold2 25 -mu 2 -output result_250_100_25 
    
    11. (b) Open ../codes/filtering.py

    filtering.py filters out the words that appreas more threshold times in history, and appears more than threshold1 times before the previous year and appears more than threshold2 times in the interval years. Also, in order to filte out the new words that immediately appeared in some time, we use mu to control the new words. Any words' appearing time between the interval years multiply mu should be smaller than the appearing time before the previous year.

    Run it:
        python filtering.py -hi ../accmulate_document_fre/2016 -th 250 -fr ../accmulate_document_fre/ -th1 100 -th2 50 -o ../filtering_250_100_50/

    Where   -hi represents the historical document frequency;
            -th represents the threshold; -fr represents the accumulated document frequency's diretory;
            -th1 represents threshold1; -th2 represents threshold2; -mu represents mu;
            -o represents output directory.

    12. Open ../codes/simplify.py

    simplify.py scans the words in concatenated_org, aiming at filters out the unstable words whose frequency are very small.

    The result will be put in director ../embeddings/after_filtering_concatenated/

    Run it:
        python simplify.py -i ../embeddings/concatenated_org -hi ../accmulate_document_fre/2016 -th 250 -fr ../document_frequency/ -th1 25 -o ../embeddings/after_filtering_concatenated/

    13. Open ../codes/k-nearest-graph/Linux/main.cpp

    main.cpp and LargeVis.cpp circulates the k-nearest graph of the input embeddings.

    You can see more information on: https://github.com/lferry007/LargeVis

    Here I modify some places in order to fit our project. The mainly changes are that the input of my program is a directory that contains many different years' embeddings and the output is also a directory.

    You can compile it through:
        g++ LargeVis.cpp main.cpp -o LargeVis -lm -pthread -lgsl -lgslcblas -Ofast -march=native -ffast-math -I /storage6/foreseer/users/zhuofeng/.local/include/ -L /storage6/foreseer/users/zhuofeng/.local/lib/

    Run it:
        ./LargeVis -input ../../../embeddings/after_filtering_concatenated -output ../../../ann-graph
#   Attention: The nestest node in ANN results has the largest value.

#   You can also use python knn_simple_version.py -i ../embeddings/after_filtering_concatenated -o ../knn-graph -k 150
#   to double check the results of ANN

    14. Open ../codes/knn+jacaard_dis.py

    knn+jaccard_dis.py compares two periods' k-nearest graph to get the results.

    Its input is a knn-graph directory and a filtering directory that filters out the useless words. And it puts the results in a file.

    Run it by:
        python knn+jaccard_dis.py -i ../knn-graph_500_100_50 -hi ../accmulate_document_fre/2016 -th 500 -d ../filtering_5_500_100_50/ -k 50 -y 5 -p 50 -o knn+jaccard_dis_5_500_100_50.txt

    14. Open ../codes/knn+bayesian_not_output_parameters.py 

    knn+bayesian_not_output_parameters.py compares two periods' k-nearest graph to get the results by using Bayesian Surprise.

    Its input is also a knn-graph directory and a filtering directory that filters out the useless words in each training year. And it puts the results in a file.

    Run it by:
        python knn+bayesian_not_output_parameters.py -i ../knn-graph -o knn+bayesian_5_500_100_50.txt -d ../filtering_1_500_100_50 -k 50 -y 5 -p 50 -t 2000 -hi ../accmulate_document_fre/2016 -th 500

#    15. pre_change_words.cpp filters out the words that appears more than threshold times across the whole history, and have already appeared more than threshold1 times before current times, and appears more than threshold2 times this years. In order to filter out the words that are immediately appeared, we use mu = 2 to control the change of words' frequency. Where threshold, threshold1, threshold2 are parameters that we input in.
#
#    In order to simplify the knn.py, we make a copy of each years' data after filtering and put it into a directory called 'filtering'.
#
#    We can also choose the words that we want to replace in file modify.txt
#
#    You can compile it through:
#        g++-4.6 pre_change_words.cpp -o pre_change_words.o
#
#    Run it by:
#        ./pre_change_words.o -frequency accmulate_document_fre -threshold 500 -threshold1 50 -threshold2 25 -output_file modify.txt -output_dir filtering
#
