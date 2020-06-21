/*
 * csim.c - A cache simulator
 *
 * The simulator accepts memory trace and outputs the number of
 * hits, misses, evictions, dirty bytes evicted and dirty bytes in cache.
 *
 * Author: enhanc
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>

#include "cachelab.h"

typedef struct CacheLine {
	int dirtyFlag;
	int validFlag;
	int tag;
	int block;
	int lruCounter;
} CacheLine;

typedef struct Result {
	int hits;
	int misses;
	int evictions;
	int dirtyBytesEvicted;
	int dirtyBytesInCache;
} Result;

char *tracePtr;
int s;
int b;
int E;
// CacheLine **cache;
// Result *result;

void parseInput(int argc, char **argv);
CacheLine **initCache();
Result *startTrace(CacheLine **cache);
void readCache(unsigned long long addr, CacheLine **cache, Result *result);
void writeCache(unsigned long long addr, CacheLine **cache, Result *result);
int getTag(unsigned long long addr);
int getSet(unsigned long long addr);
void freeCache();

int main(int argc, char **argv) {

        parseInput(argc, argv);
        
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
        freeCache(cache);
        return 0;
}

void parseInput(int argc, char **argv) {
	int opt = 0;
	while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
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
			tracePtr = optarg;
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
	CacheLine **cache = (CacheLine **) calloc(1 << s, sizeof(CacheLine*));
	
	if(!cache) {
		return NULL;
	}
	// TODO handle malloc fails
	int i;
	for (i = 0; i < (1 << s); i++ ) {
		cache[i] = (CacheLine*) calloc( E, sizeof(CacheLine));
	}
	return cache;
}

Result *startTrace(CacheLine **cache) {
	FILE *fptr = fopen(tracePtr, "r");

	if (!fptr) {
		return NULL;
	}

	Result *result = (Result*) calloc(1, sizeof(Result));

	if (!result) {
		return NULL;
	}

	char op;
	unsigned long long addr;
	int size;

	while (fscanf(fptr, " %c %llx %d", &op, &addr, &size) > 0) {
		switch(op) {
			case 'L':
			//TODO read from cache
			readCache(addr,cache,result);

			break;
			case 'S':
			// TODO write to cache
			writeCache(addr,cache,result);
			break;
			default:
			break;
		}
	}

	fclose(fptr);

	return result;
}

void readCache(unsigned long long addr, CacheLine **cache, Result *result) {
	int setIndex = getSet(addr);
	int tag = getTag(addr);
	int i;
	for (i = 0; i < E; i ++) {
		// check for hit
		if (cache[setIndex][i].validFlag && 
			cache[setIndex][i].tag == tag) {
			++(cache[setIndex][i].lruCounter);
			++(result->hits); 
		} 
		return;
	}

	// check for cold miss
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].validFlag == 0) {
			cache[setIndex][i].validFlag = 1;
			cache[setIndex][i].tag = tag;
			cache[setIndex][i].lruCounter = 1;
			++(result->misses);
			return;
		}
	}

	// conflict miss
	int minUseIndex = -1;
	int minUseCount = INT_MAX;
	for (int i = 0; i < E; i++) {
		if (cache[setIndex][i].lruCounter < minUseCount) {
			minUseCount = cache[setIndex][i].lruCounter;
			minUseIndex = i;
		}
	}

	if (minUseIndex >= 0) {
		if(cache[setIndex][minUseIndex].dirtyFlag) {
			result->dirtyBytesEvicted += (1 << b);
			result->dirtyBytesInCache -= (1 << b);
		}
		cache[setIndex][minUseIndex].validFlag = 1;
		cache[setIndex][minUseIndex].tag = tag;
		cache[setIndex][minUseIndex].lruCounter = 1;
		++(result->evictions);
		++(result->misses);
	}
	return;
}


void writeCache(unsigned long long addr, CacheLine **cache, Result *result) {
	int setIndex = getSet(addr);
	int tag = getTag(addr);
	int i = 0;
	//write hit: write back
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].validFlag &&
			cache[setIndex][i].tag == tag) {
			cache[setIndex][i].dirtyFlag = 1;
			++(cache[setIndex][i].lruCounter);
			result->dirtyBytesInCache += (1 << b);
			++(result->hits);
			return;
		}
	}

	//write miss: write allocate

	// find empty line to load data into cache
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].validFlag == 0) {
			cache[setIndex][i].validFlag = 1;
			cache[setIndex][i].tag = tag;
			cache[setIndex][i].dirtyFlag = 1;
			cache[setIndex][i].lruCounter = 1;
			++(result->misses);
			result->dirtyBytesInCache += (1 << b);
			return;
		}
	}

	// no empty line, replace one to make space
	int minUseIndex = -1;
	int minUseCount = INT_MAX;
	for (int i = 0; i < E; i++) {
		if (cache[setIndex][i].lruCounter < minUseCount) {
			minUseCount = cache[setIndex][i].lruCounter;
			minUseIndex = i;
		}
	}

	if (minUseIndex >= 0) {
		if(cache[setIndex][minUseIndex].dirtyFlag) {
			result->dirtyBytesEvicted += (1 << b);
			result->dirtyBytesInCache -= (1 << b);
		}
		cache[setIndex][minUseIndex].validFlag = 1;
		cache[setIndex][minUseIndex].tag = tag;
		cache[setIndex][minUseIndex].lruCounter = 1;
		cache[setIndex][minUseIndex].dirtyFlag = 1;
		++(result->evictions);
		++(result->misses);
		result->dirtyBytesInCache += (1 << b);
	}
	return;

}

int getSet(unsigned long long addr) {
	if (s == 0) {
		return s;
	}
	return ((1 << s) - 1) & (addr >> b);
}

int getTag(unsigned long long addr) {
	return ((1 << (64 - b - s))- 1) & (addr >> (b + s));
}

// int getBlock(unsigned long long addr) {
// 	if (b == 0) {
// 		return addr;
// 	}

// 	return ((1 << b) - 1) & (addr)
// }

void freeCache(CacheLine** cache) {
	// TODO handle free cache and result
	int i;
	for (i = 0; i < (1 << s); i++) {
		free(cache[i]);
	}
	free(cache);
}



