#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <dirent.h>
#include <unistd.h>

#define MAX_STRING 100

const int hash_table_size = 30000000;

//typedef float real;                    // Precision of float numbers

struct ClassVertex {
	double degree;
	char *name;
};

char vector_file1[MAX_STRING], vector_file2[MAX_STRING], output_file[MAX_STRING];
char directory_1[MAX_STRING], directory_2[MAX_STRING], output_directory[MAX_STRING];
struct ClassVertex *vertex;
int binary = 0;
int *vertex_hash_table;
long long max_num_vertices = 1000, num_vertices = 0;
long long vector_dim1, vector_dim2;
double *vec1, *vec2;

/* Build a hash table, mapping each vertex name to a unique vertex id */
unsigned int Hash(char *key)
{
	unsigned int seed = 131;
	unsigned int hash = 0;
	while (*key)
	{
		hash = hash * seed + (*key++);
	}
	return hash % hash_table_size;
}

void InitHashTable()
{
	vertex_hash_table = (int *)malloc(hash_table_size * sizeof(int));
	for (int k = 0; k != hash_table_size; k++) vertex_hash_table[k] = -1;
}

void InsertHashTable(char *key, int value)
{
	int addr = Hash(key);
	while (vertex_hash_table[addr] != -1) addr = (addr + 1) % hash_table_size;
	vertex_hash_table[addr] = value;
}

int SearchHashTable(char *key)
{
	int addr = Hash(key);
	while (1)
	{
		if (vertex_hash_table[addr] == -1) return -1;
		if (!strcmp(key, vertex[vertex_hash_table[addr]].name)) return vertex_hash_table[addr];
		addr = (addr + 1) % hash_table_size;
	}
	return -1;
}

/* Add a vertex to the vertex set */
int AddVertex(char *name, int vid)
{
	int length = strlen(name) + 1;
	if (length > MAX_STRING) length = MAX_STRING;
	vertex[vid].name = (char *)calloc(length, sizeof(char));
	strcpy(vertex[vid].name, name);
	vertex[vid].degree = 0;
	InsertHashTable(name, vid);
	return vid;
}

void ReadVector()
{
	char ch, name[MAX_STRING];
	double f_num;
	long long l;

	FILE *fi = fopen(vector_file1, "rb");
	if (fi == NULL) {
		printf("Vector file 1 not found\n");
		exit(1);
	}
	fscanf(fi, "%lld %lld", &num_vertices, &vector_dim1);
	vertex = (struct ClassVertex *)calloc(num_vertices, sizeof(struct ClassVertex));
	vec1 = (double *)calloc(num_vertices * vector_dim1, sizeof(double));
	for (long long k = 0; k != num_vertices; k++)
	{
		fscanf(fi, "%s", name);
		ch = fgetc(fi);
		AddVertex(name, k);
		l = k * vector_dim1;
		for (int c = 0; c != vector_dim1; c++)
		{
//			fread(&f_num, sizeof(double), 1, fi);
			fscanf(fi, "%lf", &f_num);
			vec1[c + l] = (double)f_num;
		}
	}
	fclose(fi);

	fi = fopen(vector_file2, "rb");
	if (fi == NULL) {
		printf("Vector file 2 not found\n");
		exit(1);
	}
	fscanf(fi, "%lld %lld", &l, &vector_dim2);
	vec2 = (double *)calloc((num_vertices + 1) * vector_dim2, sizeof(double));
	for (long long k = 0; k != num_vertices; k++)
	{
		fscanf(fi, "%s", name);
		ch = fgetc(fi);
		int i = SearchHashTable(name);
		if (i == -1) l = num_vertices * vector_dim2;
		else l = i * vector_dim2;
		for (int c = 0; c != vector_dim2; c++)
		{
//			fread(&f_num, sizeof(double), 1, fi);
			fscanf(fi, "%lf", &f_num);
			vec2[c + l] = (double)f_num;
		}
	}
	fclose(fi);
}


void TrainModel() {
	long long a, b;
	double len;

	InitHashTable();
	ReadVector();

	FILE *fo;
	fo = fopen(output_file, "wb");
	fprintf(fo, "%lld %lld\n", num_vertices, vector_dim1 + vector_dim2);
	for (a = 0; a < num_vertices; a++) {
		fprintf(fo, "%s ", vertex[a].name);

		len = 0;
		for (b = 0; b < vector_dim1; b++) len += vec1[b + a * vector_dim1] * vec1[b + a * vector_dim1];
		for (b = 0; b < vector_dim2; b++) len += vec2[b + a * vector_dim2] * vec2[b + a * vector_dim2];
		len = sqrt(len);

		for (b = 0; b < vector_dim1; b++) vec1[b + a * vector_dim1] /= len;

		for (b = 0; b < vector_dim2; b++) vec2[b + a * vector_dim2] /= len;

		if (binary)
		{
			for (b = 0; b < vector_dim1; b++)
				fwrite(&vec1[a * vector_dim1 + b], sizeof(double), 1, fo);
			for (b = 0; b < vector_dim2; b++)
				fwrite(&vec2[a * vector_dim2 + b], sizeof(double), 1, fo);
		}
		else
		{
			for (b = 0; b < vector_dim1; b++)
				fprintf(fo, "%lf ", vec1[a * vector_dim1 + b]);
			for (b = 0; b < vector_dim2; b++)
				fprintf(fo, "%lf ", vec2[a * vector_dim2 + b]);
		}
		fprintf(fo, "\n");
	}
	fclose(fo);
}

int ArgPos(char *str, int argc, char **argv) {
	int a;
	for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
		if (a == argc - 1) {
			printf("Argument missing for %s\n", str);
			exit(1);
		}
		return a;
	}
	return -1;
}

int main(int argc, char **argv) {
	int i;
	char current_dir[MAX_STRING];
	if (argc == 1) {
		printf("Concatenate the 1st-order embedding and the 2nd-order embeddings\n\n");
		printf("Options:\n");
		printf("Parameters for training:\n");
		printf("\t-input1 <file>\n");
		printf("\t\tThe 1st-order embeddings\n");
		printf("\t-input2 <file>\n");
		printf("\t\tThe 2nd-order embeddings\n");
		printf("\t-output <file>\n");
		printf("\t\tUse <file> to save the concatenated embeddings\n");
		printf("\t-binary <int>\n");
		printf("\t\tSave the learnt embeddings in binary moded; default is 0 (off)\n");
		printf("\nExamples:\n");
		printf("./concatenate.o -input1 vec_1st -input2 vec_2nd -output vec_all -binary 0\n\n");
		return 0;
	}
	if ((i = ArgPos((char *)"-input1", argc, argv)) > 0) strcpy(directory_1, argv[i + 1]);
	if ((i = ArgPos((char *)"-input2", argc, argv)) > 0) strcpy(directory_2, argv[i + 1]);
	if ((i = ArgPos((char *)"-output", argc, argv)) > 0) strcpy(output_directory, argv[i + 1]);
	if ((i = ArgPos((char *)"-binary", argc, argv)) > 0) binary = atoi(argv[i + 1]);
	
	DIR *pdir;
	struct dirent *pent;
	pdir = opendir(current_dir);
	if (pdir == NULL) {
		printf("Opendir %s failure!\n", directory_1);
		exit(1);
	}
    if (directory_1[strlen(directory_1) - 1] != "/")
        strcat(directory_1, "/");
    if (directory_2[strlen(directory_2) - 1] != "/")
        strcat(directory_2, "/");
    if (output_directory[strlen(output_directory) - 1] != "/")
        strcat(output_directory, "/");

	while ((pent = readdir(pdir)) != NULL) {
		if (strcmp(pent->d_name, ".") != 0 && strcmp(pent->d_name, "..") != 0 && strcmp(pent->d_name, "README.md") != 0) {
			strcpy(vector_file1, directory_1);
			strcat(vector_file1, pent->d_name);

			strcpy(vector_file2, directory_2);
			strcat(vector_file2, pent->d_name);

			strcpy(output_file, output_directory);
			strcat(output_file, pent->d_name);

			TrainModel();
			
			free(vertex_hash_table);
			free(vec1);
			free(vec2);
		}
	}


	return 0;
}
