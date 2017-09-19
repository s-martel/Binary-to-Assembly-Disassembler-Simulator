#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
#include "Stats.h"

using namespace std;

CacheStats::CacheStats() {
	cout << "Cache Config: ";
	if (!CACHE_EN) {
		cout << "cache disabled" << endl;
	}
	else {
		cout << (SETS * WAYS * BLOCKSIZE) << " B (";
		cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
		cout << "  Latencies: Hit = " << HIT_LATENCY << " cycles, ";
		cout << "Read = " << READ_LATENCY << " cycles, ";
		cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
	}

	reads = 0;
	writes = 0;
	read_misses = 0;
	write_misses = 0;
	writebacks = 0;

	/* TODO: your code here */

	for (int i = 0; i < SETS; ++i) {  // initialize all blocks to invalid
		for (int j = 0; j < WAYS; ++j) {
			cacheSet[i][j].dirtyBit = 0;
			cacheSet[i][j].validBit = 0;
			cacheSet[i][j].tag = -1;
		}
		roundRobin[i] = 0;  // initialize all round robins
	}
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
	if (!CACHE_EN) { // no cache
		return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
	}

	/* TODO: your code here */

	int set = (addr / BLOCKSIZE) % SETS; // compute set index
	int tag = (addr / BLOCKSIZE) / SETS; // compute tag  (this should be addr/(SETS*BLOCKSIZE), but it didn't seem to work
	int way = roundRobin[set];  // set WAY to use according to set's round robin
	int latency = HIT_LATENCY;

	if (type == LOAD) ++reads;
	if (type == STORE) ++writes;

	for (int i = 0; i < WAYS; ++i) {  // if HIT
		if (cacheSet[set][i].validBit && cacheSet[set][i].tag == tag) {
			if (type == STORE) cacheSet[set][i].dirtyBit = true;  // set dirty bit if STORE
			return latency;
		}
	}

	latency += READ_LATENCY;  // if MISS
	(type == LOAD) ? ++read_misses : ++write_misses;

	if (cacheSet[set][way].dirtyBit) {  // if WRITEBACK
		++writebacks;
		latency += WRITE_LATENCY;
	}

	cacheSet[set][way].validBit = true;
	cacheSet[set][way].dirtyBit = (type == STORE) ? true : false;
	cacheSet[set][way].tag = tag;
	roundRobin[set] = (roundRobin[set] + 1) % WAYS;  // increment round robin for this set

	return latency;
}

void CacheStats::printFinalStats() {
	/* TODO: your code here (don't forget to drain the cache of writebacks) */
	for (int i = 0; i < SETS; ++i) {
		for (int j = 0; j < WAYS; ++j)
			if (cacheSet[i][j].dirtyBit == 1) ++writebacks;  // write backs
	}

	int accesses = reads + writes;
	int misses = read_misses + write_misses;
	cout << "Accesses: " << accesses << endl;
	cout << "  Reads: " << reads << endl;
	cout << "  Writes: " << writes << endl;
	cout << "Misses: " << misses << endl;
	cout << "  Read misses: " << read_misses << endl;
	cout << "  Write misses: " << write_misses << endl;
	cout << "Writebacks: " << writebacks << endl;
	cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
	cout << "%" << endl;
}
