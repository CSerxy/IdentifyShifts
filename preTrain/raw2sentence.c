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
} Vertex;

#define MAX_STRING 101
#define MAX_HASH 30000000
#define MAX_MAP 70000 

Vertex *vertex;
int *hash_table;
const char * punc = ".?\'\";:!,#=~%&|/$";
const char * punc1 = "-{}[]()<>";
char input_file[MAX_STRING], output_file[MAX_STRING], vocabulary_file[MAX_STRING];
char input_dir[MAX_STRING], output_dir[MAX_STRING], vocabulary_dir[MAX_STRING];
FILE *fin, *fout, *fvo;
int num_edges = 0, num_vertices = 0, max_num_vertices = 1000;	
char voc[MAX_MAP + 1][MAX_STRING];
int num_voc;

int ArgPos(char* str, int argc, char** argv);
void VocabularyInput();
void Input();
void Output();
int deal(char* current, int *i);
void Add(char *a, char *b, double value);
unsigned int Hash(char* key);
void InitHashTable();
void InitMap();
void InsertHashTable(char* key, int value);
int SearchHashTable(char* key);
int AddVertex(char* name);

int main(int argc, char** argv){
	int i;
	if ((i = ArgPos((char*)"-i", argc, argv)) > 0)
		strcpy(input_dir, argv[i+1]);
	if ((i = ArgPos((char*)"-v", argc, argv)) > 0)
		strcpy(vocabulary_dir, argv[i+1]);
	if ((i = ArgPos((char*)"-o", argc, argv)) > 0)
		strcpy(output_dir, argv[i+1]);
	char temp[MAX_STRING];

	DIR *dirp;
	struct dirent *direntp;
	if ((dirp = opendir(input_dir)) == NULL) {
		printf("Open Directory input Error: %s\n", strerror(errno));
		exit(1);
	}

	while ((direntp = readdir(dirp)) != NULL) {
		if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
printf("%s\t", direntp->d_name);
fflush(stdout);
			strcpy(input_file, input_dir);
			strcat(input_file, direntp->d_name);
			strcpy(output_file, output_dir);
			strcat(output_file, direntp->d_name);
			strcat(output_file, ".ngrams");
			strcpy(vocabulary_file, vocabulary_dir);
                        strcat(vocabulary_file, direntp->d_name);

			fin = fopen(input_file, "r");
			fout = fopen(output_file, "w");
			fvo = fopen(vocabulary_file, "r");
			
			if (fin == NULL || fvo == NULL) {
				printf("Open input file error!\n");
				exit(0);
			}
			InitHashTable();
			vertex = (Vertex*)calloc(max_num_vertices, sizeof(Vertex));	
			
			VocabularyInput();
			Input();

			free(vertex);
			free(hash_table);
			num_vertices = 0;
			num_edges = 0;
			num_voc = 0;
			max_num_vertices = 1000;

			fclose(fin);
			fclose(fout);
			fclose(fvo);
		}
	}
	return 0;
}

void VocabularyInput(){
	num_voc = 0;
	char useless[100];
	while (fscanf(fvo, "%s", voc[num_voc]) != EOF && num_voc < MAX_MAP){
		fgets(useless, 100, fvo);
		AddVertex(voc[num_voc++]);
	}
}

void Input(){
	int last, right, judge;
	char current[200][MAX_STRING];
	char temp[100000];
	while (1){
		last = 1;
		right = 0;
		if (fscanf(fin, "%s", temp) == EOF)
			break;
		if (strlen(temp) < MAX_STRING)
			strcpy(current[right % 200], temp);
		else
			continue;
		if (strcmp(current[right % 200], "<start>") == 0){
			while (1){
				fscanf(fin, "%s", temp);
				if (strlen(temp) < MAX_STRING)
					strcpy(current[(right = right + 1) % 200], temp);
				else
					continue;
				if (strcmp(current[right % 200], "<end>") == 0)
					break;
				if (deal(current[right % 200], &right) == 1){
					for (int i = last; i <= right; i++)
					if (SearchHashTable(current[i % 200]) != -1) {
						fprintf(fout, "%s ", current[i % 200]);
/*						fprintf(fout, "%s", current[i % 200]);
						for (int j = i + 1; j <= i + 4 && j <= right; j++) 
						if (SearchHashTable(current[j % 200]) != -1)
							fprintf(fout, " %s", current[j % 200]);
						fprintf(fout, "\n");
*/					}
					fprintf(fout, "\n");
					last = right + 1;
				}
			}
			right--;
			if (right > last){
				for (int i = last; i <= right; i++)
					if (SearchHashTable(current[i % 200]) != -1) {
						fprintf(fout, "%s ", current[i % 200]);
/*						for (int j = i + 1; j <= i + 4 && j <= right; j++) 
						if (SearchHashTable(current[j % 200]) != -1)
							fprintf(fout, " %s", current[j % 200]);
						fprintf(fout, "\n");
*/					}
				fprintf(fout, "\n");
			}
		}
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
	if (num_vertices == MAX_MAP){
		printf("Error: Too much words!\n");
		exit(1);
	}
	int len = strlen(name) + 1;
	if (len > MAX_STRING)
		len = MAX_STRING;
	vertex[num_vertices].name = (char *)calloc(len, sizeof(char));
	strcpy(vertex[num_vertices].name, name);

	if (num_vertices + 3 >= max_num_vertices){						/* may have bugs, not sure */
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
