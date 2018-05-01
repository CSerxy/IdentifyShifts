#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <pthread.h>

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>

using namespace std;

typedef struct Vertex{
	char* name;
	double degree;
} Vertex;

#define MAX_HASH 30000000
#define MAX_STRING 100
#define NEG_SAMPLING_POWER 0.75
#define SIGMOID_BOUND 6

char network_file[MAX_STRING], embedding_file[MAX_STRING], last_embedding_file[MAX_STRING];
char network_dir[MAX_STRING], embedding_dir[MAX_STRING];
Vertex* vertex;
int* edges_source_id, *edges_target_id;
double* edges_weight;
int* hash_table;
int num_edges = 0, num_vertices = 0, old_num_vertices = 0, max_num_vertices = 1000;
int is_binary = 0, order = 2, dim = 100, is_first_training = 1;
int num_negative = 5, num_threads = 1;
long long total_samples = 1, current_sample_count = 0;
double rho, init_rho = 0.025;
double* emb_vertex, *emb_context;
int neg_table_size = 1e8;
int* neg_table;
double* sigmoid_table;
int sigmoid_table_size = 1000;
char date[3600][8];

const gsl_rng_type* gsl_T;
gsl_rng* gsl_r;

long long* alias;
double* prob;

unsigned int Hash(char* key);
void InitHashTable();
void InsertHashTable(char* key, int value);
int SearchHashTable(char* key);
int AddVertex(char* name);
void Input();
int ArgPos(char* str, int argc, char** argv);
void TrainLINE();
void InitAliasTable();
long long SampleAnEdge(double value1, double value2);
void InitVector();
void InitSigmoidTable();
void Update(double* vec1, double* vec2, double* error, int label);
double FastSigmoid(double u);
int Rand(unsigned long long& seed);
void Output();
void InitNegTable();
void* TrainLINEThread(void *id);
int compar(char* a, char* b);
void swap(char* a, char* b);

int main(int argc, char** argv){
	int i;
	if (argc == 1) {
		printf("LINE: Large Information Network Embedding\n\n");
		printf("Options:\n");
		printf("Parameters for training:\n");
		printf("\t-train <file>\n");
		printf("\t\tUse network data from <file> to train the model\n");
		printf("\t-output <file>\n");
		printf("\t\tUse <file> to save the learnt embeddings\n");
		printf("\t-binary <int>\n");
		printf("\t\tSave the learnt embeddings in binary moded; default is 0 (off)\n");
		printf("\t-size <int>\n");
		printf("\t\tSet dimension of vertex embeddings; default is 100\n");
		printf("\t-order <int>\n");
		printf("\t\tThe type of the model; 1 for first order, 2 for second order; default is 2\n");
		printf("\t-negative <int>\n");
		printf("\t\tNumber of negative examples; default is 5\n");
		printf("\t-samples <int>\n");
		printf("\t\tSet the number of training samples as <int>Million; default is 1\n");
		printf("\t-threads <int>\n");
		printf("\t\tUse <int> threads (default 1)\n");
		printf("\t-rho <float>\n");
		printf("\t\tSet the starting learning rate; default is 0.025\n");
		printf("\nExamples:\n");
		printf("./line_init.o -binary 0 -size 100 -order 2 -negative 5 -samples 100 -rho 0.025 -threads 16\n\n");
		return 0;
	}
	if ((i = ArgPos((char *)"-train", argc, argv)) > 0) strcpy(network_dir, argv[i + 1]);
	if ((i = ArgPos((char *)"-output", argc, argv)) > 0) strcpy(embedding_dir, argv[i + 1]);
	if ((i = ArgPos((char *)"-binary", argc, argv)) > 0) is_binary = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-size", argc, argv)) > 0) dim = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-order", argc, argv)) > 0) order = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-negative", argc, argv)) > 0) num_negative = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-samples", argc, argv)) > 0) total_samples = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-rho", argc, argv)) > 0) init_rho = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);

    char strtemp[MAX_STRING];
    strcpy(strtemp, "../");
    strcat(strtemp, network_dir);
    strcat(strtemp, "/");
    strcpy(network_dir, strtemp);

    strcpy(strtemp, "../embeddings/");
    strcat(strtemp, embedding_dir);
    strcat(strtemp, "/");
    strcpy(embedding_dir, strtemp);

	total_samples *= 1000000;
	rho = init_rho;
	vertex = (Vertex*)calloc(max_num_vertices, sizeof(Vertex));
	if (order != 1 && order != 2){
		printf("Error: order should be 1 or 2!\n");
		exit(1);
	}

	InitHashTable();
	InitSigmoidTable();

	DIR *dirp;
	struct dirent *direntp;
	if ((dirp = opendir(network_dir)) == NULL) {
		printf("Open Direntory input Error: %s\n", strerror(errno));
		exit(1);
	}
	
	int count = 0;
	while ((direntp = readdir(dirp)) != NULL) {
		if (strlen(direntp->d_name) == 4) {
			strcpy(date[count ++], direntp->d_name);
		}
	}

	for (int i = 0; i < count - 1; i++)
		for (int j = i + 1; j < count; j++)
		if (compar(date[i], date[j]) < 0)
			swap(date[i], date[j]);
	int temp = 0;
	while (temp != count) {
		strcpy(network_file, network_dir);
		strcat(network_file, date[temp]);
		strcpy(embedding_file, embedding_dir);
		strcat(embedding_file, date[temp]);
		Input();
		InitVector();
		InitAliasTable();
		InitNegTable();
		TrainLINE();
		free(edges_source_id);
		free(edges_target_id);
		free(edges_weight);
		free(alias);
		free(prob);
		free(neg_table);
		for (long long b = 0; b < dim; b++) 
		for (long long a = 0; a < num_vertices; a++)
			emb_context[a * dim + b] = 0;
		strcpy(last_embedding_file, embedding_file);
		temp ++;
		current_sample_count = 0;
        old_num_vertices = num_vertices;
	}
	return 0;
}

void TrainLINE(){
	pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

	printf("--------------------------------\n");
	printf("Order: %d\n", order);
	printf("Samples: %lldM\n", total_samples / 1000000);
	printf("Negative: %d\n", num_negative);
	printf("Dimension: %d\n", dim);
	printf("Initial rho: %lf\n", init_rho);
	printf("--------------------------------\n");

	gsl_rng_env_setup();
	gsl_T = gsl_rng_rand48;
	gsl_r = gsl_rng_alloc(gsl_T);
	gsl_rng_set(gsl_r, 314159265);
	clock_t start = clock();
	printf("--------------------------------\n");
	for (long i = 0;i < num_threads;i++)
		pthread_create(&pt[i], NULL, TrainLINEThread, (void *)i);
	for (long i = 0;i < num_threads;i++)
		pthread_join(pt[i], NULL);
	printf("\n");
	clock_t finish = clock();
	printf("Total time: %lf\n",(double)(finish - start) / CLOCKS_PER_SEC);
	Output();
	free(pt);
}

void* TrainLINEThread(void *id){
	long long a, b, aa, bb, target, label, edge;
	long long count = 0, last_count = 0;
	unsigned long long seed = (long long)id;
	double* vec_error;
	vec_error = (double*)calloc(dim, sizeof(double));
	if (vec_error == NULL){
		printf("Error: memory allocation failed!\n");
		exit(1);
	}
	while (1){
		if (count > total_samples / num_threads + 2)
			break;
		if (count - last_count > 10000){
			current_sample_count += count - last_count;
			last_count = count;
			printf("%cRho: %lf  Progress: %.3lf%%", 13, rho, (double)current_sample_count / (double)(total_samples + 1) * 100);
			fflush(stdout);
			rho = init_rho * (1 - current_sample_count / (double)(total_samples + 1));
			if (rho < init_rho * 0.0001)
				rho = init_rho * 0.0001;
		}
		edge = SampleAnEdge(gsl_rng_uniform(gsl_r),gsl_rng_uniform(gsl_r));
		a = edges_source_id[edge];
		b = edges_target_id[edge];
		aa = a * dim;
		for (int i = 0;i != dim;i++)
			vec_error[i] = 0;
		for (int i = 0;i != num_negative + 1;i++){
			if (i == 0){
				target = b;
				label = 1;
			} else {
				target = neg_table[Rand(seed)];
				label = 0;
			}
			bb = target * dim;
			if (order == 1)
				Update(&emb_vertex[aa], &emb_vertex[bb], vec_error, label);
			if (order == 2)
				Update(&emb_vertex[aa], &emb_context[bb], vec_error, label);
		}
		for (int i = 0;i != dim;i++)
			emb_vertex[i + aa] += vec_error[i];
		count++;
	}
	free(vec_error);
	pthread_exit(NULL);
}

void Output()
{
	FILE *fo = fopen(embedding_file, "wb");
	if (fo == NULL) {
		printf("Open output file error!\n");
		fflush(stdout);
		exit(0);
	}

	fprintf(fo, "%d %d\n", num_vertices, dim);
	for (int i = 0; i < num_vertices; i++)
	{
		fprintf(fo, "%s ", vertex[i].name);
		if (is_binary){
			for (int j = 0; j < dim; j++) 
				fwrite(&emb_vertex[i * dim + j], sizeof(double), 1, fo);
		}
		else{
		       for (int j = 0; j < dim; j++) 
				fprintf(fo, "%lf ", emb_vertex[i * dim + j]);
		}
		fprintf(fo, "\n");
	}
	fclose(fo);
}

int Rand(unsigned long long& seed){
	seed = seed * 25214903917 + 11;
	return (seed >> 16) % neg_table_size;
}

void Update(double* vec1, double* vec2, double* vec_error, int label){
	double u = 0, uu;
	for (int i = 0;i != dim;i++)
		u += vec1[i] * vec2[i];
	uu = (label - FastSigmoid(u)) * rho;
	for (int i = 0;i != dim;i++)
		vec_error[i] += uu * vec2[i];
	for (int i = 0;i != dim;i++)
		vec2[i] += uu * vec1[i];
}

double FastSigmoid(double u){
	if (u > SIGMOID_BOUND)
		return 1;
	else if (u < -SIGMOID_BOUND)
		return 0;
	int k = (u + SIGMOID_BOUND) * sigmoid_table_size / SIGMOID_BOUND / 2;
	return sigmoid_table[k];
}

void InitSigmoidTable(){
	double x;
	sigmoid_table = (double*)malloc((sigmoid_table_size + 1) * sizeof(double));
	for (int i = 0;i != sigmoid_table_size;i++){
		x = 2 * SIGMOID_BOUND * i / sigmoid_table_size - SIGMOID_BOUND;
		sigmoid_table[i] = 1 / (1 + exp(-x));
	}
}

void InitNegTable(){
	int id = 0;
	double sum = 0, value = 0, cur_sum = 0;
	neg_table = (int*)malloc(neg_table_size * sizeof(int));
	for (int i = 0;i != num_vertices;i++)
		sum += pow(vertex[i].degree, NEG_SAMPLING_POWER);
	for (int i = 0;i != neg_table_size;i++){
		if ((double)(i + 1) / neg_table_size > value){
			cur_sum += pow(vertex[id].degree, NEG_SAMPLING_POWER);
			value = cur_sum / sum;
			id++;
		}
		neg_table[i] = id - 1;
	}
}

long long SampleAnEdge(double value1, double value2){
	long long u = (long long)num_edges * value1;
	if (value2 < prob[u])
		return u;
	else
		return alias[u];
}

void InitVector(){
	long long a, b;
	if (is_first_training) {
		emb_vertex = (double*)calloc(dim * num_vertices, sizeof(double));
		if (emb_vertex == NULL){ printf("Error: memory allocation failed\n"); exit(1); }
		for (b = 0; b < dim; b++) 
			for (a = 0; a < num_vertices; a++)
				emb_vertex[a * dim + b] = (rand() / (double)RAND_MAX - 0.5) / dim;
	
		emb_context = (double*)calloc(dim * num_vertices, sizeof(double));
		if (emb_context == NULL){ printf("Error: memory allocation failed\n"); exit(1); }
		for (b = 0; b < dim; b++) 
			for (a = 0; a < num_vertices; a++)
				emb_context[a * dim + b] = 0;	
		is_first_training = 0;
	} else {
		emb_vertex = (double*)realloc(emb_vertex, num_vertices * dim * sizeof(double));
		emb_context = (double*)realloc(emb_context, num_vertices * dim * sizeof(double));
		for (b = 0; b < dim; b++)
			for (a = old_num_vertices; a < num_vertices; a++)
				emb_vertex[a * dim + b] = (rand() / (double)RAND_MAX - 0.5) / dim;

		for (b = 0; b < dim; b++)
			for (a = 0; a < num_vertices; a++)
				emb_context[a * dim + b] = 0;
	}
}

void InitAliasTable(){
	alias = (long long*)malloc(num_edges * sizeof(long long));
	prob = (double*)malloc(num_edges * sizeof(double));
	if (alias == NULL || prob == NULL){
		printf("Error: memory allocation failed!\n");
		exit(1);
	}

	double* norm_prob = (double*)malloc(num_edges * sizeof(double));
	long long* large_block = (long long*)malloc(num_edges * sizeof(long long));
	long long* small_block = (long long*)malloc(num_edges * sizeof(long long));
	if (norm_prob == NULL || large_block == NULL || small_block == NULL){
		printf("Error: memory allocation failed!\n");
		exit(1);
	}

	double sum = 0;
	long long cur_small_block, cur_large_block;
	long long num_small_block = 0, num_large_block = 0;

	for (long long i = 0;i != num_edges;i++)
		sum += edges_weight[i];
	for (long long i = 0;i != num_edges;i++)
		norm_prob[i] = edges_weight[i] * num_edges / sum;

	for (long long i = num_edges - 1;i >= 0;i--){
		if (norm_prob[i] < 1)
			small_block[num_small_block++] = i;
		else
			large_block[num_large_block++] = i;
	}

	while (num_small_block && num_large_block){
		cur_small_block = small_block[--num_small_block];
		cur_large_block = large_block[--num_large_block];
		prob[cur_small_block] = norm_prob[cur_small_block];
		alias[cur_small_block] = cur_large_block;
		norm_prob[cur_large_block] = norm_prob[cur_large_block] + norm_prob[cur_small_block] - 1;
		if (norm_prob[cur_large_block] < 1)
			small_block[num_small_block++] = cur_large_block;
		else
			large_block[num_large_block++] = cur_large_block;
	}

	while (num_small_block)
		prob[small_block[--num_small_block]] = 1;
	while (num_large_block)
		prob[large_block[--num_large_block]] = 1;
	free(norm_prob);
	free(small_block);
	free(large_block);
}

int ArgPos(char* str, int argc, char** argv){
	for (int i = 1;i < argc;i++)
	if (strcmp(str, argv[i]) == 0){
		if (i == argc - 1){
			printf("Argument missing for %s\n", str);
			exit(1);
		}
		return i;
	}
	return -1;	
}

void Input(){
	FILE* fin;

	fin = fopen(network_file, "r");
	if (fin == NULL){
		printf("ERROR: network file not found!\n");
		exit(1);
	}
	num_edges = 0;
	char str[2 * MAX_STRING + 10000];				/* Because each line has two name string and a double number */
	while (fgets(str, sizeof(str), fin))
		num_edges++;
	fclose(fin);

	edges_source_id = (int*)malloc(num_edges * sizeof(int));
	edges_target_id = (int*)malloc(num_edges * sizeof(int));
	edges_weight = (double*)malloc(num_edges * sizeof(double));
	if (edges_source_id == NULL || edges_target_id == NULL || edges_weight == NULL){
		printf("ERROR: memory allocation failed!\n");
		exit(1);
	}

	char name1[MAX_STRING], name2[MAX_STRING];
	double weight;
	int id;
	fin = fopen(network_file, "r");
	for (int i = 0;i < num_edges;i++){
		fscanf(fin, "%s%s%lf", name1, name2, &weight);
		
		id = SearchHashTable(name1);
		if (id == -1)
			id = AddVertex(name1);
		edges_source_id[i] = id;
		vertex[id].degree += weight;

		id = SearchHashTable(name2);
		if (id == -1)
			id = AddVertex(name2);
		edges_target_id[i] = id;
		vertex[id].degree += weight;

		edges_weight[i] = weight;
	}
	fclose(fin);
}

int AddVertex(char* name){
	int len = strlen(name) + 1;
	if (len > MAX_STRING)
		len = MAX_STRING;
	vertex[num_vertices].name = (char *)calloc(len, sizeof(char));
	strcpy(vertex[num_vertices].name, name);
	vertex[num_vertices].degree = 0;

	if (num_vertices + 3>= max_num_vertices){						/* may have bugs, not sure */
		max_num_vertices += 1000;
		vertex = (Vertex*)realloc(vertex, max_num_vertices * sizeof(Vertex));
	}
	InsertHashTable(vertex[num_vertices].name, num_vertices);
	return num_vertices++;
}

unsigned int Hash(char* key){
	unsigned int re = 0;
	while (*key){
		re = (re * 131 + (*key++)) % MAX_HASH;
	}
	return re;
}

void InitHashTable(){
	hash_table = (int*)malloc(MAX_HASH * sizeof(int));
	for (int i = 0;i != MAX_HASH;i++)
		hash_table[i] = -1;
}

void InsertHashTable(char* key, int value){
	unsigned int addr = Hash(key);
	while (hash_table[addr] != -1){
		addr = (addr + 1) % MAX_HASH;
	}
	hash_table[addr] = value;
}

int SearchHashTable(char* key){
	unsigned int addr = Hash(key);
	while (hash_table[addr] != -1){
		if (strcmp(vertex[hash_table[addr]].name, key) == 0)
			return hash_table[addr];
		addr = (addr + 1) % MAX_HASH;
	}
	return -1;
}

int compar(char * a, char * b) {
	for (int i = 0; i < strlen(a); i++) {
		if (a[i] < b[i])
			return 1;
		if (a[i] > b[i])
			return -1;
	}
	return 0;
}

void swap(char * a, char * b) {
	char c[8];
	strcpy(c, a);
	strcpy(a, b);
	strcpy(b, c);
}
