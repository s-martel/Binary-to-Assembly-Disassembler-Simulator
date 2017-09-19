#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include "Debug.h"
using namespace std;

enum PIPESTAGE {
	IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5,
	MEM2 = 6, WB = 7, PIPESTAGES = 8
};

class Stats {
private:
	long long cycles;
	int flushes;
	int bubbles;
	int stalls;

	int memops;
	int branches;
	int taken;

	int rawhazards[PIPESTAGES];

	int resultReg[PIPESTAGES];
	int resultStage[PIPESTAGES];

public:
	Stats();

	void clock(PIPESTAGE stage);

	void flush(int count);

	void registerSrc(int r, PIPESTAGE needed);
	void registerDest(int r, PIPESTAGE available);

	void countMemOp() { memops++; }
	void countBranch() { branches++; }
	void countTaken() { taken++; }

	// getters
	long long getCycles() { return cycles; }
	int getFlushes() { return flushes; }
	int getBubbles() { return bubbles; }
	int getMemOps() { return memops; }
	int getBranches() { return branches; }
	int getTaken() { return taken; }
	int getStalls() { return stalls; }

	/*	int getRAWhazards() { return RAWhaz; }*/
	int getRAWhazards(PIPESTAGE stage);
	void stall(int cycles);

private:
	void bubble();

};

#endif




