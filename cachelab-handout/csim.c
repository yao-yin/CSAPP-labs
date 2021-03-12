#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

typedef unsigned long long uint64_t;

int hits = 0;
int misses = 0;
int evictions = 0;
int verbose = 0;
int s, E, b;
FILE * fp;
typedef struct {
    int lru, valid;
    uint64_t tag;
} Cacheline;

typedef Cacheline* CacheSet;
typedef CacheSet* Cache;
Cache cache;

const char* usage = "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n";

void parseArgs(int argc, char * argv[]);
void simulate();
int visitCache(uint64_t address);

int main(int argc, char *argv[])
{
    parseArgs(argc, argv);
    simulate();
    printSummary(hits, misses, evictions);
    return 0;
}

void parseArgs(int argc, char * argv[]){
    // parse arguments, chage s, E, b;
    int opt;
    while ((opt = getopt(argc, argv,"hvs:E:b:t:"), opt != -1)) {
        /*
        h: help flag;
        v: verbose mode;
        s, E, b: arguments
        t: tracefile;
        */
        switch (opt) {
            case 'h':
                fprintf(stdout, usage, argv[0]);
                exit(1);
            case 'v':
                verbose = 1;
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
            case 't':
                fp = fopen(optarg, "r");
                break;
            default:
                fprintf(stdout, usage, argv[0]);
                exit(1);
       }
    }
}

void simulate() {
    int S = (1 << s);
    cache = (Cache) malloc(sizeof(CacheSet)*S);
    if (cache == NULL) {return;}
    for (int i = 0; i < S; i ++) {
        cache[i] = (CacheSet) calloc(E, sizeof(CacheSet));
        if (cache[i] == NULL) {return;}
    }
    char buffer[20];
    char op;
    int sz;
    int ret = -1;
    uint64_t addr;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (buffer[0] == 'I') continue;
        else {
            sscanf(buffer, " %s %llx,%d", & op, & addr, &sz);
            switch (op) {
                case 'S':
                    ret = visitCache(addr);
                    break;
                case 'L':
                    ret = visitCache(addr);
                    break;
                case 'M':
                    ret = visitCache(addr);
                    hits ++;
                    break;
            }
        }
        if (verbose) {
            switch(ret) {
                case 0:
                    printf("%c %llx,%d hits\n", op, addr, sz);
                    break;
                case 1:
                    printf("%c %llx,%d miss\n", op, addr, sz);
                    break;
                case 2:
                    printf("%c %llx,%d eviction\n", op, addr, sz);
                    break;
            }
        }
    }
    for (int i = 0; i < S; i ++) {
        free(cache[i]);
    }
    free(cache);
    fclose(fp);
}

int visitCache(uint64_t addr) {
    uint64_t tag = addr >> (s + b);
    uint64_t setIndex = (addr >> b) & (( 1<< s) - 1);
    int evictIndex = 0;
    int empty = -1;
    CacheSet cacheSet = cache[setIndex];
    for (int i = 0; i < E; i ++) {
        if (cacheSet[i].valid) {
            if (cacheSet[i].tag == tag) {
                hits ++;
                cacheSet[i].lru = 1;
                return 0;
            }
            cacheSet[i].lru ++;
            if(cacheSet[i].lru > cacheSet[evictIndex].lru) {
                evictIndex = i;
            }
        } else {
            empty = i;
        }
    }
    misses ++;
    if (empty != -1) {
        cacheSet[empty].valid = 1;
        cacheSet[empty].lru = 1;
        cacheSet[empty].tag = tag;
        return 1;
    } else {
        cacheSet[evictIndex].tag = tag;
        cacheSet[evictIndex].lru = 1;
        evictions ++;
        return 2;
    }

}