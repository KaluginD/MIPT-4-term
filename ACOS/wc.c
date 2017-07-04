#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct opt_prms {
	int lines;
	int words;
	int bytes;
};

static struct option const long_opts[] = {
	{"lines", no_argument, NULL, 'l'},
	{"words", no_argument, NULL, 'w'},
	{"bytes", no_argument, NULL, 'c'},
	{NULL,    0,           NULL, 0}
};

void dflt_prms(struct opt_prms* prms);
void init_prms(struct opt_prms* prms);
void print_prms(struct opt_prms* prms, struct opt_prms* required);
void work(int flag, FILE* file, char* file_name, struct opt_prms* prms, 
		  struct opt_prms* ttl_prms, struct opt_prms* required);
void wc(int num_fls, char* fls[], struct opt_prms* prms);

int main(int argc, char* argv[]) {
	int c;
	int dflt = 1;
	struct opt_prms* prms = (struct opt_prms*) malloc(sizeof(struct opt_prms));
	while ((c = getopt_long(argc, argv, "lwc", long_opts, NULL)) != -1) {
		switch (c) {
			case 'l':
				prms->lines = 1;
				dflt = 0;
				break;
			case 'w':
				prms->words = 1;
				dflt = 0;
				break;
			case 'c':
				prms->bytes = 1;
				dflt = 0;
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}
	if (dflt) {
		dflt_prms(prms);
	}
	wc(argc - optind, argv + optind, prms);
	free(prms);
	return 0;
}

void dflt_prms(struct opt_prms* prms) {
	prms->lines = 1;
	prms->words = 1;
	prms->bytes = 1;
}

void init_prms(struct opt_prms* prms) {
	prms->lines = 0;
	prms->words = 0;
	prms->bytes = 0;
}

void print_prms(struct opt_prms* prms, struct opt_prms* required) {
	if (required->lines) {
		printf("\t%d", prms->lines);
	}
	if (required->words) {
		printf("\t%d", prms->words);
	}
	if (required->bytes) {
		printf("\t%d", prms->bytes);
	}
}

void work(int flag, FILE* file, char* file_name, struct opt_prms* prms, 
		  struct opt_prms* ttl_prms, struct opt_prms* required) {
	char buf[80];
	int space = 1;
	struct stat sb;
	while (fgets(buf, sizeof(buf), file) != NULL) {
		for (int i = 0; i < strlen(buf); ++i) {
            if (buf[i] == '\n') {
                ++prms->lines;
                space = 1;
            }
            if (space && !isspace(buf[i])) {
                ++prms->words;
                space = 0;
            }
            if (!space && isspace(buf[i])) {
                space = 1;
			}
		}
	}
	if (fstat(fileno(file), &sb) != 0) {
		exit(EXIT_FAILURE);
	} else {
		prms->bytes = sb.st_size;
	}
	print_prms(prms, required);
	printf("\t%s\n", file_name);
	ttl_prms->lines += prms->lines;
	ttl_prms->words += prms->words;
	ttl_prms->bytes += prms->bytes;
	if (!flag) {
		fclose(file);
	}
}

void wc(int num_fls, char* fls[], struct opt_prms* prms) {
	int flag = 0;
	if (num_fls == 0) {
		++num_fls;
		flag = 1;
	}
	struct opt_prms* ttl_prms = (struct opt_prms*) malloc(sizeof(struct opt_prms));
	init_prms(ttl_prms);
	for (int i = 0; i < num_fls; ++i) {
		struct opt_prms* curr_prms = (struct opt_prms*) malloc(sizeof(struct opt_prms));
		init_prms(curr_prms);	
		FILE* file;
		if (flag) {
			file = stdin;
		} else {
			file = fopen(fls[i], "r");
		}
		if (!file) {
			exit(EXIT_FAILURE);
		}
		work(flag, file, fls[i], curr_prms, ttl_prms, prms);
		free(curr_prms);
	}
	if (num_fls > 1) {
		print_prms(ttl_prms, prms);
		printf("\t%s\n", "total");
	}
	free(ttl_prms);
}