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
	unsigned long long tag;
	int block;
	int lruCounter;
} CacheLine;

typedef struct Result {
	int hits;
	int misses;
	int evictions;
	int totalDirtyCount;
	int evictedDirtyCount;
} Result;

char *tracePtr;
int s;
int b;
int E;
int verboseFlag = 0;
int timeStamp = 0;
// CacheLine **cache;
// Result *result;

void parseInput(int argc, char **argv);
CacheLine **initCache();
Result *startTrace(CacheLine **cache);
void readCache(unsigned long long addr, CacheLine **cache, Result *result);
void writeCache(unsigned long long addr, CacheLine **cache, Result *result);
unsigned long long getTag(unsigned long long addr);
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
        	int dirtyBytesInCache = (1 << b) * ((result->totalDirtyCount) - (result->evictedDirtyCount));
        	int dirtyBytesEvicted = (1 << b) * (result->evictedDirtyCount);

        	printSummary(result->hits, 
        		result->misses, 
        		result->evictions, 
        		dirtyBytesInCache,
        		dirtyBytesEvicted);
        }
        freeCache(cache);
        return 0;
}

void parseInput(int argc, char **argv) {
	int opt = 0;
	while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
			case 'v':
			verboseFlag = 1;
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
		++timeStamp;
		switch(op) {
			case 'L':
			readCache(addr,cache,result);
			break;
			case 'S':
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
	unsigned long long tag = getTag(addr);
	int i;

	for (i = 0; i < E; i ++) {
		// check for hit
		if (cache[setIndex][i].validFlag && 
			cache[setIndex][i].tag == tag) {
			cache[setIndex][i].lruCounter = timeStamp;
			++(result->hits); 
			if (verboseFlag) {
				printf("L %llx hit\n", addr);
			}
			return;
		} 
	}

	// check for cold miss
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].validFlag == 0) {
			cache[setIndex][i].validFlag = 1;
			cache[setIndex][i].tag = tag;
			cache[setIndex][i].lruCounter = timeStamp;
			cache[setIndex][i].dirtyFlag = 0;
			++(result->misses);
			if (verboseFlag) {
				printf("L %llx miss\n", addr);
			}
			return;
		}
	}

	// conflict miss
	int minUseIndex = -1;
	int minUseCount = INT_MAX;
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].lruCounter < minUseCount) {
			minUseCount = cache[setIndex][i].lruCounter;
			minUseIndex = i;
		}
	}
	if (minUseIndex >= 0) {
		if(cache[setIndex][minUseIndex].dirtyFlag) {
			cache[setIndex][minUseIndex].dirtyFlag = 0;
			++(result->evictedDirtyCount);
		}
		cache[setIndex][minUseIndex].validFlag = 1;
		cache[setIndex][minUseIndex].tag = tag;
		cache[setIndex][minUseIndex].lruCounter = timeStamp;
		++(result->evictions);
		++(result->misses);
		if (verboseFlag) {
			printf("L %llx miss eviction\n", addr);
		}
	}
	return;
}


void writeCache(unsigned long long addr, CacheLine **cache, Result *result) {
	int setIndex = getSet(addr);
	unsigned long long tag = getTag(addr);
	int i = 0;

	//write hit: write back
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].validFlag &&
			cache[setIndex][i].tag == tag) {
			if (cache[setIndex][i].dirtyFlag == 0) {
				++(result->totalDirtyCount);
				cache[setIndex][i].dirtyFlag = 1;
			}
			cache[setIndex][i].lruCounter = timeStamp;
			++(result->hits);
			if (verboseFlag) {
				printf("S %llx hit\n", addr);
			}
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
			cache[setIndex][i].lruCounter = timeStamp;
			++(result->misses);
			++(result->totalDirtyCount);
			if (verboseFlag) {
				printf("S %llx miss\n", addr);
			}
			return;
		}
	}

	// no empty line, replace one to make space
	int minUseIndex = -1;
	int minUseCount = INT_MAX;
	for (i = 0; i < E; i++) {
		if (cache[setIndex][i].lruCounter < minUseCount) {
			minUseCount = cache[setIndex][i].lruCounter;
			minUseIndex = i;
		}
	}

	if (minUseIndex >= 0) {
		if(cache[setIndex][minUseIndex].dirtyFlag) {
			++(result->evictedDirtyCount);
			cache[setIndex][minUseIndex].dirtyFlag = 0;
		}
		cache[setIndex][minUseIndex].validFlag = 1;
		cache[setIndex][minUseIndex].tag = tag;
		cache[setIndex][minUseIndex].lruCounter = timeStamp;
		cache[setIndex][minUseIndex].dirtyFlag = 1;
	 	++(result->totalDirtyCount);
		++(result->evictions);
		++(result->misses);
			if (verboseFlag) {
				printf("S %llx miss, eviction\n", addr);
			}
	}
	return;

}

int getSet(unsigned long long addr) {
	if (s == 0) {
		return s;
	}
	return ((1 << s) - 1) & (addr >> b);
}

unsigned long long getTag(unsigned long long addr) {
	unsigned long long shift = b+s;
	return ((1LL << (63LL - shift))- 1LL) & (addr >> shift);
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



