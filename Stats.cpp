#include "Stats.h"

Stats::Stats() {
	cycles = PIPESTAGES - 1; // pipeline startup cost
	flushes = 0;
	bubbles = 0;
	stalls = 0;

	memops = 0;
	branches = 0;
	taken = 0;

	// 	RAWhaz = 0;
	// 	EXE1haz = 0;
	// 	EXE2haz = 0;
	// 	MEM1haz = 0;
	// 	MEM2haz = 0;

	for (int i = IF1; i < PIPESTAGES; i++) {
		/*resultReg[i] = -1;*/
		rawhazards[i] = 0;
		resultReg[i] = -1;
		resultStage[i] = 0;
	}
}

void Stats::clock(PIPESTAGE stage) {
	cycles++;

	// pipeline the register tracking from provided start stage
	// (ops in 'stage' thru end of pipe advance, ops in stages before 'stage'
	// are frozen)
	for (int i = WB; i > stage; i--) {
		resultReg[i] = resultReg[i - 1];
		resultStage[i] = resultStage[i - 1];
	}
	// inject no-op into 'stage'
	resultReg[stage] = -1;
	resultStage[stage] = 0;
}

void Stats::registerSrc(int r, PIPESTAGE stage) {
	if (r == 0)
		return;

	for (int i = EXE1; i < WB; ++i) {
		if (r == resultReg[i]) {
			int needed = stage - ID;
			int ready = resultStage[i] - i;
			++rawhazards[i];

			while (ready > needed) {
				bubble();
				--ready;
			}
			break;
		}
	}
}

void Stats::registerDest(int r, PIPESTAGE stage) {
	resultReg[ID] = r;   // insert instance of register write into pipeline
	resultStage[ID] = stage;
}

void Stats::flush(int count) { // count == how many ops to flush
	for (int i = 0; i < count; ++i) {
		++flushes;
		clock(IF1);
	}
}

int Stats::getRAWhazards(PIPESTAGE stage) {
	if (stage < PIPESTAGES)
		return rawhazards[stage];

	int total = 0;
	for (int i = 0; i < PIPESTAGES; ++i) {
		total += rawhazards[i];
	}
	return total;
}

void Stats::bubble() {
	++bubbles;
	clock(EXE1);
}

void Stats::stall(int cycles) {
	for (int i = 0; i < cycles; ++i) {
		clock(WB);
		++stalls;
	}
}
