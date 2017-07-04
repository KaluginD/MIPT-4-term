#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ftail (FILE *fp) {
	int begin = 0;
	int num = 0;
	char *buffer;
	char line[256];
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	buffer = malloc((len + 1) * sizeof(char));
	int i = 0;
	for (; i < len && num < 10; ++i) {
		fseek(fp, len - 1 - i, SEEK_SET);
		buffer[i] = fgetc(fp);
		if(buffer[i] == 10) {
			if(i != 0) {
				int j = 0;        
				for(int loop = i; loop > begin; loop--) {
					if((buffer[loop] == 10) && (j == 0))
						continue;
					line[j] = buffer[loop];
					++j;
				}
				line[j] = 0;
				begin = i;
				printf("%s\n",line);
				++num;
			}
		}
	}
	if(i > begin && num < 10) {
		int j = 0;
		for(int loop = i; loop > begin; --loop) {       
			if((j == 0) && ((buffer[loop] == 10) || (buffer[loop] == 0)))
				continue;    
			line[j] = buffer[loop];
			j++;
		}
		line[j] = 0;
		printf("%s\n",line);
		return;
	}
}

int main(int argc, char** argv) {
	if (argc == 1) {
		ftail(stdin);
	}
	else if (argc == 2) {
		FILE* f;
		if(f = fopen(argv[1], "r")) {
				ftail(f);
				fclose(f);
			}
			else 
				perror(argv[1]);
	}
	else {
		for (int i = 1; i < argc; ++i) {
			FILE* f;
			if(f = fopen(argv[i], "r")) {
				printf("%s\n", argv[i]);
				ftail(f);
				fclose(f);
			}
			else 
				perror(argv[i]);
		}
	}
	return 0;
}
