/*
 * csim.c - A cache simulator
 *
 * The simulator accepts memory trace and outputs the number of
 * hits, misses, evictions, dirty bytes evicted and dirty bytes in cache.
 *
 * Author: enhanc
 */

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "cachelab.h"

typedef struct CacheLine {
	int dirtyFlag = 0;
	int validFlag = 0;
	long tag = 0;
	long block = 0;
	int lruCounter = 0;
} CacheLine;

char *tracePtr;
int s;
int b;
int E;
CacheLine **cache;

int main(int argc, const char **argv) {
        parseInput(argc, argv);
        initCache();
        
        // TODO print correct value
        printSummary(0, 0, 0, 0, 0);
        return 0;
}

void parseInput(int argc, const char **argv) {

	while(opt = getopt(argc, argv, "hvs:E:b:t:") != -1) {
		switch (opt) {
			case 'v':
			// TODO verbose version
			break;
			case 's':
			s = atoi(optarg);
			break;
			case 'E':
			E = atoi(optarg);
			break;
			case 'b':
			b = atoi(optarg);
			break;
			case't':
			if (optarg) {
				tracePtr = optarg
			}
			break;
			case'h':
			default:
			//help
			break;

		}
	}
	return;
} 

void initCache() {
	cache = (CacheLine **) malloc(1 << s * sizeof(CacheLine*));
	for (i = 0; i < s; i++ ) {
		cache[i] = (CacheLine*) malloc(1 << E * sizeof(CacheLine));
	}
	return;
}

void freeCache() {
	for (i = 0; i < s; i++) {
		free(cache[i])
	}
	free(cache);
}


