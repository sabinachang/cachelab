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

typedef struct Result {
	int hits = 0;
	int misses = 0;
	int evictions = 0;
	int dirtyBytesEvicted = 0;
	int dirtyBytesInCache = 0;
} Result;

char *tracePtr;
int s;
int b;
int E;
// CacheLine **cache;
// Result *result;

int main(int argc, const char **argv) {

        parseInput(argc, argv);
        
        // TODO print correct value
        CacheLine **cache = initCache();
        if (!cache) {
        	return 0;
        }
        
        Result *result = startTrace(cache);

        if (result) {
        	printSummary(result->hits, 
        		result->misses, 
        		result->evictions, 
        		result->dirtyBytesEvicted, 
        		result->dirtyBytesInCache);
        }
        freeCache();
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

CacheLine **initCache() {
	CacheLine **cache = (CacheLine **) malloc(1 << s * sizeof(CacheLine*));
	
	if(!cache) {
		return NULL;
	}
	// TODO handle malloc fails
	for (i = 0; i < s; i++ ) {
		cache[i] = (CacheLine*) malloc(1 << E * sizeof(CacheLine));
	}
	return cache;
}

Result *startTrace(CacheLine **cache) {
	FILE *fptr = fopen(tracePtr, "r");

	if (!fptr) {
		return NULL;
	}

	Result *result = (Result*) malloc(sizof(Result));

	if (!result) {
		return NULL;
	}

	char op;
	unsigned long addr;
	int size;

	while (fscanf(fptr, " %c %lx %d", &op, &addr, &size) > 0) {
		switch(op) {
			case 'L':
			//TODO read from cache
			break;
			case 'S':
			// TODO write to cache
			break;
			default:
			break;
		}
	}

	fclose(fptr);

	return;
}



void freeCache() {
	// TODO handle free cache and result
	for (i = 0; i < s; i++) {
		free(cache[i]);
	}
	free(cache);
}




