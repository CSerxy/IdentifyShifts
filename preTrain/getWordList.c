#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

typedef struct Vertex{
	char *name;
	double value;
} Vertex;

#define MAX_STRING 101
#define MAX_HASH 30000000

Vertex *vertex, *vertex2;
int *hash_table, *hash_table2;
const char * punc = ".?\'\";:!,#=~%&|/$";
const char * punc1 = "-{}[]()<>";
double weight = 0.25;
char input_dir[MAX_STRING], output_dir[MAX_STRING], input_file[MAX_STRING], output_file[MAX_STRING];
FILE *fin, *fout;
int num_edges = 0, num_vertices = 0, max_num_vertices = 1000, num_vertices2 = 0, max_num_vertices2 = 1000;	
char current[2000][MAX_STRING];
char temp[100000];
long long count = 0;
	
int ArgPos(char* str, int argc, char** argv);
void Input();
void Output();
int deal(char* current, int *i);
void Add(char *a, char *b, double value);
unsigned int Hash(char* key);
void InitHashTable();
void InsertHashTable(char* key, int value);
int SearchHashTable(char* key);
void InitHashTable2();
void InsertHashTable2(char* key, int value);
int SearchHashTable2(char* key);
int AddVertex(char* name);
int AddVertex2(char* name);
void AddEdges(char *a, char *b, double value);
int cmp(const void * a, const void * b);

int main(int argc, char** argv){
	int i;
	if ((i = ArgPos((char*)"-i", argc, argv)) > 0)
		strcpy(input_dir, argv[i+1]);
	if ((i = ArgPos((char*)"-o", argc, argv)) > 0)
		strcpy(output_dir, argv[i+1]);

    if (input_dir[strlen(input_dir) - 1] != '/')
        strcat(input_dir, "/");
    if (output_dir[strlen(output_dir) - 1] != '/')
        strcat(output_dir, "/");

	DIR *dirp;
	struct dirent *direntp;
	if ((dirp = opendir(input_dir)) == NULL) {
		printf("Open Directory input Error: %s\n", strerror(errno));
		exit(1);
	}
	InitHashTable2();
	vertex2 = (Vertex*)calloc(max_num_vertices2, sizeof(Vertex));

	while ((direntp = readdir(dirp)) != NULL) {
		if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
            printf("%s\t", direntp->d_name);
            fflush(stdout);

			strcpy(input_file, input_dir);
			strcat(input_file, direntp->d_name);
			strcpy(output_file, output_dir);
			strcat(output_file, direntp->d_name);

			fin = fopen(input_file, "r");
			fout = fopen(output_file, "w");
			if (fin == NULL || fout == NULL) {
				printf("Open file %s or %s error!\n", input_file, output_file);
				exit(0);
			}

			InitHashTable();
			vertex = (Vertex*)calloc(max_num_vertices, sizeof(Vertex));
			Input();
			qsort(vertex, num_vertices, sizeof(Vertex), cmp);
			Output();

			free(vertex);
			free(hash_table);
			num_vertices = 0;
			num_edges = 0;
			max_num_vertices = 1000;

			fclose(fin);
			fclose(fout);
		}
	}
	printf("%lld\n", count);
	return 0;
}

int cmp(const void * a, const void * b){
	Vertex * c = (Vertex *)a;
	Vertex * d = (Vertex *)b;
	if (c->value < d->value) return 1;
	if (c->value == d->value) return 0;
	if (c->value > d->value) return -1;
	return c->value < d->value;
}

void Output(){
	if (num_vertices > 100000)
		num_vertices = 100000;
	int i;
	for (i = 0; i < num_vertices; i++) 
	if (vertex[i].value >= 10 && strlen(vertex[i].name) >= 2 && strlen(vertex[i].name) <= 15) {
		fprintf(fout, "%s %.2lf\n", vertex[i].name, vertex[i].value);
		free(vertex[i].name);
	}
}

void AddEdges(char *a, char *b, double value){
	int id1, id2;
	id1 = SearchHashTable(a);
	if (id1 == -1)
		id1 = AddVertex(a);
	id2 = SearchHashTable(b);
	if (id2 == -1)
		id2 = AddVertex(b);
	
	if (SearchHashTable2(a) == -1) {
		AddVertex2(a);
		vertex[id1].value += 1;
	}

	if (SearchHashTable2(b) == -1) {
		AddVertex2(b);
		vertex[id2].value += 1;
	}

//	printf("%s %.2lf %s %.2lf\t", vertex[id1].name, vertex[id1].value, vertex[id2].name, vertex[id2].value);
//	fflush(stdout);

//	vertex[id1].value += 1;
//	vertex[id2].value += 1;
}

void Input(){
	int last, right, judge;
//int count = 0;
	while (1){
		last = 1;
		right = 0;
		if (fscanf(fin, "%s", temp) == EOF)
			break;
		if (strlen(temp) <= 15 && strlen(temp) >= 2)
			strcpy(current[right % 2000], temp);
		else
			continue;
		if (strcmp(current[right % 2000], "<start>") == 0){
			while (1){
				fscanf(fin, "%s", temp);
				if (strlen(temp) <= 15 && strlen(temp) >= 2)
					strcpy(current[(++right) % 2000], temp);
				else
					continue;
				if (strcmp(current[(right) % 2000], "<end>") == 0)
					break;
				if (deal(current[right % 2000], &right) == 1){
					for (int i = last; i <= right; i++) {
						int id = SearchHashTable(current[i % 2000]);
						if (id == -1)
							id = AddVertex(current[i % 2000]);
						vertex[id].value += 1;
						count++;
					}
					last = right + 1;
				}
			}
			right--;
			if (right > last){
				for (int i = last; i <= right; i++) {
					int id = SearchHashTable(current[i % 2000]);
					if (id == -1)
						id = AddVertex(current[i % 2000]);
					vertex[id].value += 1;
					count++;
				}
			}
//			printf("%.2lf ", vertex[1].value);
//			fflush(stdout);
			for (int i = 0; i < num_vertices2; i++) {
				if (vertex2[i].name == NULL)
					continue;

				int k = SearchHashTable2(vertex2[i].name);
				if (k == -1)
					continue;
				while (hash_table2[k] != -1) {
					free(vertex2[hash_table2[k]].name);
					hash_table2[k] = -1;
					k = (k + 1) % MAX_HASH;
				}

			}
			num_vertices2 = 0;
//			printf("4\n");
//			fflush(stdout);
		}
//		fgets(temp, 1000, fin);
	}
}

int deal(char* current, int *i){
	int j, k;
    int mark;
    mark = 1;
    if (strlen(current) == 1) {
        *i = *i - 1;
        return 0;
    }
    if (current[0] == '(') {
        for (j = 1; j < strlen(current); j++) {
            if (!((current[j] <= 122 && current[j] >= 97) || (current[j] <= 90 && current[j] >= 65))) {
                *i = *i - 1;
                return 0;
            }
            if (current[j] <= 90)
                current[j] += 32;
        }
        for (j = 1; j < strlen(current); j++)
            current[j - 1] = current[j];
        current[strlen(current) - 1] = '\0';
        return 2;
    }

	for (j = 0; j < strlen(current) - 1; j++){
		if (!((current[j] <= 122 && current[j] >= 97) || (current[j] <= 90 && current[j] >= 65))){
            mark = 0;
		} 
		if (current[j] <= 90 && current[j] >= 65)
			current[j] += 32;
	}
    if (mark == 0)
        *i = *i - 1;
	if (current[j] <= 90 && current[j] >= 65)
		current[j] += 32;
	if (current[j] <= 122 && current[j] >= 97){
		current[j + 1] = '\0';
		return 2 * mark;
	}
	else if (strchr(punc, current[j]) != NULL && j != 0){
		current[j] = '\0';
        if (current[j - 1] == ')') {
            for (k = 0; k < j - 1; k++) {
                if (!((current[k] <= 122 && current[k] >= 97) || (current[k] <= 90 && current[k] >= 65)))
                    return 0;
            }
            current[j - 1] = '\0';
            if (mark == 0 && strlen(current) != 0)
                *i = *i + 1;
        }
		return 1;
	} else if (strchr(punc1, current[j]) != NULL && j != 0) {
        if (mark != 0) {
           current[j] = '\0';
           return 2;
        }
        return 0;
    } else { 
        if (mark != 0)
            *i = *i - 1;
		return 0;
    }
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

int AddVertex(char* name){
	int len = strlen(name) + 1;
	if (len > MAX_STRING)
		len = MAX_STRING;
	vertex[num_vertices].name = (char *)calloc(MAX_STRING, sizeof(char));
	strcpy(vertex[num_vertices].name, name);
	vertex[num_vertices].value = 0;

	if (num_vertices + 3 >= max_num_vertices){						/* may have bugs, not sure */
		max_num_vertices += 1000;
		vertex = (Vertex*)realloc(vertex, max_num_vertices * sizeof(Vertex));
	}
	InsertHashTable(vertex[num_vertices].name, num_vertices);
	return num_vertices++;
}

int AddVertex2(char* name) {
	int len = strlen(name) + 1;
	if (len > MAX_STRING)
		len = MAX_STRING;
	vertex2[num_vertices2].name = (char *)calloc(len, sizeof(char));
	strcpy(vertex2[num_vertices2].name, name);

	if (num_vertices2 + 3 >= max_num_vertices2) {
		max_num_vertices2 += 1000;
		vertex2 = (Vertex*)realloc(vertex2, max_num_vertices2 * sizeof(Vertex));
	}
	InsertHashTable2(vertex2[num_vertices2].name, num_vertices2);
	return num_vertices2++;
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

void InitHashTable2(){
	hash_table2 = (int*)malloc(MAX_HASH * sizeof(int));
	for (int i = 0;i != MAX_HASH;i++)
		hash_table2[i] = -1;
}

void InsertHashTable2(char* key, int value){
	unsigned int addr = Hash(key);
	while (hash_table2[addr] != -1){
		addr = (addr + 1) % MAX_HASH;
	}
	hash_table2[addr] = value;
}

int SearchHashTable2(char* key){
	unsigned int addr = Hash(key);
	while (hash_table2[addr] != -1){
		if (strcmp(vertex2[hash_table2[addr]].name, key) == 0)
			return addr;
		addr = (addr + 1) % MAX_HASH;
	}
	return -1;
}
