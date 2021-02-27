
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include "utils.h"
#include "pubpem.h"



static int verify(char *file_fname, char *sign_fname, unsigned char *pubpem);



static void Usage()
{
	printf("%s",
			"Usage:\n" \
			"	-f|--file FILE\n" \
			"		specify verified file\n" \
			"	-s|--sign64 FILE\n" \
			"		specify sign file\n" \
			"	[-p|--pem FILE]\n" \
			"		pub pem-key\n"
	);
	exit(1);
}



int main(int argc, char **argv)
{
	char *file_fname = 0;
	char *sign_fname = 0;
	char *pem_fname = 0;
	unsigned char *pubpem = 0;

	struct option loptions[] = {
		{"file", required_argument, 0, 'f'},
		{"sign64", required_argument, 0, 's'},
		{"pem", required_argument, 0, 'p'},
		{"help", no_argument, 0, 'h' },
	  	{0, 0, 0, 0}
	};
	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "f:s:p:h", loptions, &option_index);
		if (c == -1)
   				break;

		switch (c) {
			case 'f': {
				if (optarg) {
					size_t len = strlen(optarg);
					if (len) {
						file_fname = (char*)calloc(len+1, 1);
						strcpy(file_fname, optarg);
					}
				}
			} break;
			case 's': {
				if (optarg) {
					size_t len = strlen(optarg);
					if (len) {
						sign_fname = (char*)calloc(len+1, 1);
						strcpy(sign_fname, optarg);
					}
				}
			} break;
			case 'p': {
				if (optarg) {
					size_t len = strlen(optarg);
					if (len) {
						pem_fname = (char*)calloc(len+1, 1);
						strcpy(pem_fname, optarg);
					}
				}
			} break;
			default:
				Usage();
				break;
		}
	}

	struct stat filestat;
	if ( stat(file_fname, &filestat) != 0 || stat(sign_fname, &filestat) != 0) {
		printf("Cannot get stat %s %s\n", file_fname, sign_fname);
		Usage();
	}

	/* setup public pem to pem-file or pem-bin */
	int _is_need_to_free = 0;
	if (pem_fname) {
		if ( stat(pem_fname, &filestat) != 0 ) {
			printf("Cannot get stat %s\n", pem_fname);
			exit(1);
		}
		int pem_d = open(pem_fname, O_RDONLY);
		if ( pem_d < 0 ) {
			printf("Cannot open %s\n", pem_fname);
			exit(1);
		}
		pubpem = (unsigned char*)calloc(filestat.st_size, 1);
		int rr = read(pem_d, pubpem, filestat.st_size);
		close(pem_d);
		if ( rr < filestat.st_size ) {
			free(pubpem);
			exit(1);
		}
		_is_need_to_free = 1;
	} else {
		pubpem = public_key_pem;
	}

	int res = verify(file_fname, sign_fname, pubpem);

	/* clean up */
	if (_is_need_to_free)
		free(pubpem);
	free(file_fname);
	free(sign_fname);

	return res;
}



static int verify(char *file_fname, char *sign_fname, unsigned char *pubpem)
{
	char *file_buf;
	char *sign_buf;
	size_t file_size;
	struct stat filestat;
	int file_d, sign_d;

	/* open file */
	file_d = open(file_fname, O_RDONLY, S_IRWXU);
	if ( file_d < 0 ) {
		fprintf(stderr, "Error: cannot open %s", file_fname);
		return 1;
	}

	/* get stat -> size */
	if ( stat(file_fname, &filestat) != 0 ) {
		close(file_d);
		fprintf(stderr, "Error: cannot get stat of %s", file_fname);
		return 1;
	}

	/* map file to memory */
	file_size = filestat.st_size;
	file_buf = (char*)mmap(0, file_size, PROT_READ, MAP_SHARED, file_d, 0);
	if ( file_buf == MAP_FAILED ) {
		close(file_d);
		fprintf(stderr, "Error: cannot map %s to memory", file_fname);
		return 1;
	}

	/* open sign64 */
	sign_d = open(sign_fname, O_RDONLY, S_IRWXU);
	if ( sign_d < 0 ) {
		fprintf(stderr, "Error: cannot open %s", sign_fname);
		return 1;
	}

	/* get stat -> size */
	if ( stat(sign_fname, &filestat) != 0 ) {
		close(sign_d);
		fprintf(stderr, "Error: cannot get stat of %s", sign_fname);
		return 1;
	}

	/* read sign */
	sign_buf = (char*)calloc(filestat.st_size, 1);
	if ( read(sign_d, sign_buf, filestat.st_size) <= 0 ) {
		free(sign_buf);
		close(sign_d);
		fprintf(stderr, "Error: cannot read %s", sign_fname);
		return 1;
	}

	/* verify */
	int res = verifySignature((void*)pubpem, file_buf, file_size, sign_buf);

	/* clean up */
	close(file_d);
	free(sign_buf);

	return res;
}


