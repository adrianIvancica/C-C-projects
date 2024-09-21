#include <stdlib.h>
#include <stdio.h>

struct Record {
	char ln[12];
	char fn[12];
	int fiveNum;
	char grades[7];
};
int main() {
	struct Record record;

	FILE* fin = fopen("/home/www/assign/grades.dat", "rb");
	if (fin == NULL) {
		printf("Unable to open file\n");
		exit(1);
	}

	FILE* fout = fopen("grades.txt", "w");
	if (fout == NULL) {
		printf("Unable to open file\n");
		exit(1);
	}
	
	int i = 0;
	while (1) {
		if (fread(&record, sizeof(struct Record), 1, fin) == 0) {
			break;
		}
		fprintf(fout, "%-12s %-12s %5d", record.ln, record.fn, record.fiveNum); // -12s = 12 left aligned character bytes
		for (i = 0; i < 7; i++) {
			fprintf(fout, "%3d", record.grades[i]);
		}
		fprintf(fout, "\n");
	}

	return 0;
}
