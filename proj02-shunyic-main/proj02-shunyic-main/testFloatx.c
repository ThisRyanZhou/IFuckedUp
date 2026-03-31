#include "floatx.h"
#include <stdio.h>
#include <stdlib.h> // for atoi

#ifndef NAN
#define NAN (1.0/0.0)
#endif

int main(int argc,char **argv) {

	if (argc!=3) {
		fprintf(stderr,"Invoke as %s <width> <expSize>\n",argv[0]);
		return 0;
	}

	int totBits=atoi(argv[1]);
	int expBits=atoi(argv[2]);
	// Check totBits and expBits to make sure they are valid
	if (totBits < 3 || totBits>64) {
		fprintf(stderr,"Invoke with a width of at least 3, and at most 64; not %d\n",totBits);
		return 0;
	}

	if (expBits < 1 || expBits > (totBits-2)) {
		fprintf(stderr,"Invoke with an expSize of at least 1, and at most %d; not %d\n",
			totBits-2,expBits);
		return 0;
	}

	while(!feof(stdin)) {
		double in=NAN; // NAN defined in math.h
		if(1==scanf(" %lf",&in)) {
			floatx fx = doubleToFloatx(in,totBits,expBits);
			printf("%14.8g = floatx(%d,%d) : %016lx\n",in,totBits,expBits,fx);
		} else {
			char buf[100];
			scanf(" %s",buf);
			printf("Input : %s not recognized as a double... 	quitting\n",buf);
			break;
		}
	}
	return 0;
}