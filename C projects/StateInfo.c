#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct place {
	int ncode; //numeric code
	char* state; //state abreviation
	char* name; //name of place
	int pop; //population
	double area;
	double lat; //latitude
	double lon; //longitude
	int inter; //code for representative intersection
	double dist; //distance to intersection
}place;
void placeConstructor(place* p) {
	p->ncode = -1;
	p->state = " ";
	p->name = " ";
	p->pop = -1;
	p->area = -1.0;
	p->lat = -1.0;
	p->lon = -1.0;
	p->inter = -1;
	p->dist = -1;
}
place* create_place() {
	place* temp = (place*)malloc(sizeof(place));
	placeConstructor(temp);
	return temp;
}
void printInfoOfPlace(place* p) {
	printf("code: %i\nstate: %s\nname: %s\npopulation: %i\narea: %f\nlatitude: %f\nlongitude: %f\nintersection code: %i\ndistance to intersection: %f\n", p->ncode, p->state, p->name, p->pop, p->area, p->lat, p->lon, p->inter, p->dist);
}

typedef struct link {
	struct link* next;
	struct place* data;
}link;
void linkConstructor(link* l, place* p) {
	l->next = NULL;
	l->data = p;
}
link* create_link(place* d) {
	link* temp = (link*)malloc(sizeof(link));
	linkConstructor(temp, d);
	return temp;
}

typedef struct LinkedList {
	link* first, *last;
}LL;
void LLconstructor(LL* L) {
	L->first = NULL;
	L->last = NULL;
}
LL* create_LL() {
	LL* temp = (LL*)malloc(sizeof(LL));
	LLconstructor(temp);
	return temp;
} 
void add_to_end(LL* L, place* p) {
	if (L->first == NULL && L->last == NULL) {
		L->first = create_link(p);
		L->last = L->first;
	}
	else if(L->first != NULL) {
		link* index = L->first;
		while (index->next != NULL) {
			index = index->next;
		}
		index->next = create_link(p);
		L->last = index->next;
	}
	else {
		printf("add_to_end error\n");
		exit(1);
	}
}

char* removeSpaces(char* str) {
	int i;
	int j = 0;
	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] != ' ') {
			str[j] = str[i];
			j++;
		}
	}
	str[j] = '\0';
	return str;
}
char* removeDoubleSpace(char* str) { //this function finds the FIRST instance of a double space and returns its position
	char* temp = str;
	if (strlen(str) < 2) {
		return NULL;
	}
	int i;
	int j = 0;
	for (i = 0; str[i+1] != '\0'; i++) { //single space at the end of the string is a possibility
		if (!(str[i] == ' ' && str[i+1] == ' ')) {
			temp[j] = str[i];
			j++;
		}
	}//no possibility for a double space at the end of the string after this for loop, therefore 
	if (str[j - 1] == ' ') { //removes space if its at the end of the string
		j--;
	}
	//add null character at end
	temp[j+1] = '\0';
	return str;
}
char* removeSpaceAtEnd(char* str) {
	int len = strlen(str);
	if (str[len-1] == ' ') {
		str[len-1] = '\0';
	}
	return str;
}
char* removeAllButNum(char* str) {
	int i;
	int j = 0;
	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == '0' || str[i] == '1' || str[i] == '2' || str[i] == '3' || str[i] == '4' || str[i] == '5' || str[i] == '6' || str[i] == '7' || str[i] == '8' || str[i] == '9') {
			str[j] = str[i];
			j++;
		}
	}
	//add null character at end
	str[j] = '\0';
	return str;
}

void addFile(LL* L) {
	FILE* fp = fopen("/home/www/assign/named-places.txt", "r");
	if (fp == NULL) {
		perror("Error: ");
		exit(1);
	}
	
	/*size of each column for the given data is given below*/
	//numeric code = 8
	//state abbreviation = 2
	//name, with spaces trimmed from the end, mixed with population = 56
	//area = 14
	//latitude = 10
	//longitude = 11
	//code for representative road intersection = 5
	//distance to that intersection = 8
	char buf[116]; //size of the entire line is actually 115 including newline at the end, 116 to include null character when storing to buffer
	char ncode[9];
	char state[3];
	char namePopMix[57];
	char area[15];
	char lat[11];
	char lon[12];
	char inter[6];
	char dist[9];

	while (fgets(buf, 116, fp) != NULL) {
		/*since strncpy does not automatically add null character to char* destination, '\0' is added manually to each section*/
			
		strncpy(ncode, &buf[0], 8);
		ncode[8] = '\0';
		strncpy(state, &buf[8], 2);
		state[2] = '\0';
		strncpy(namePopMix, &buf[10], 56);
		namePopMix[56] = '\0';
		strncpy(area, &buf[66], 14);
		area[14] = '\0';
		strncpy(lat, &buf[80], 10);
		lat[10] = '\0';
		strncpy(lon, &buf[90], 11);
		lon[11] = '\0';
		strncpy(inter, &buf[101], 5);
		inter[5] = '\0';
		strncpy(dist, &buf[106], 8);
		dist[8] = '\0';

		char numDelim[] = "0123456789";
		char* ptr;
		place* ptrp = create_place();

		char npmCopy[57];
		strncpy(npmCopy, namePopMix, 56);
		npmCopy[56] = '\0';
		
		ptrp->ncode = atoi(ncode); //NOTE: atoi removes leading zeros from ncode, so an ncode such as 00012345 will be saved as 12345
		ptrp->state = strdup(state);
		ptrp->name = strdup(removeSpaceAtEnd(strtok(removeDoubleSpace(namePopMix), numDelim)));
		ptrp->pop = atoi(removeAllButNum(npmCopy));
		ptrp->area = strtod(area, &ptr);
		ptrp->lat = strtod(lat, &ptr);
		ptrp->lon = strtod(lon, &ptr);
		ptrp->inter = atoi(inter);
		ptrp->dist = strtod(dist, &ptr);

		add_to_end(L, ptrp);
	}
	fclose(fp);
}
int findPlaceName(LL *L, char* plc) {
	int found = 0;
	if (L == NULL || plc == NULL) {
		printf("FPN1:Place does not exist\n");
		return 0;
	}
	struct link* index = L->first;
	if (index == NULL) {
		printf("FPN2:Place does not exist\n");
		return 0;
	}
	else {
		if (!strcmp(index->data->name, plc)) {//strcmp return 0 when both are the same
			printf("%s ", index->data->state);
			found = 1;
		}
		else {
			while (index->next != NULL) {
				index = index->next;
				if (!strcmp(index->data->name, plc)) {
					printf("%s ", index->data->state);
					found = 1;
				}
			}
		}
		if (found) {
			printf("\nPlease select a state from above: ");
		}
		else {
			printf("FPN3:Place does not exist\n");
		}
	}
	return found;
}
void findPlaceNameState(LL* L, char* plc, char* st) {
	if (L == NULL || plc == NULL || st == NULL) {
		printf("FPNS1:Place does not exist\n");
		return;
	}
	struct link* index = L->first;
	if (index == NULL) {
		printf("FPNS2:Place does not exist\n");
		return;
	}
	else {
		if (!strcmp(index->data->name, plc) && !strcmp(index->data->state, st)) {
			printInfoOfPlace(index->data);
			return;
		}
		else {
			while (index->next != NULL) {
				index = index->next;
				if (!strcmp(index->data->name, plc) && !strcmp(index->data->state, st)) {
					printInfoOfPlace(index->data);
					return;
				}
			}
			printf("Invalid state input\n");
		}
	}
}

void LOCI() {//list of commands and their info
	printf("******************************************************************************\n");
	printf("| Commands: EXIT - Exit program                                              |\n");
	printf("|           FIND - Find information about a place                            |\n");
	printf("|           HELP - Displays the list of valid commands and their information |\n");
	printf("******************************************************************************\n");
}
int main() {
	LL* List = create_LL();
	addFile(List);
	LOCI();
	
	char input[1000];
	char name[1000];
	char state[1000];
	while (1) {
		printf("Enter Command: ");
		if (fgets(input, 1000, stdin) != NULL) {
			if (strcmp(input, "EXIT\n") == 0) {
				printf("Exiting Program\n");
				exit(0);
			}
			else if (strcmp(input, "FIND\n") == 0) {
				printf("Enter place name: ");
				if (fgets(name, 1000, stdin) != NULL) {
					name[strcspn(name, "\n")] = 0; //removes '\n' character from the end of fgets buffer, used to compare string properly with database
					int exists = findPlaceName(List, name);
					if (exists) {
						if (fgets(state, 1000, stdin) != NULL) {
							state[strcspn(state, "\n")] = 0;
							findPlaceNameState(List, name, state);
						}
						else {
							printf("M:Place does not exist\n");
						}
					}
				}
				else {
					printf("Place does not exist\n");
				}
			}
			else if (strcmp(input, "HELP\n") == 0) {
				LOCI();
			}
			else {
				printf("Invalid Command\n");
			}
		}
		
	}
	return 0;
}
