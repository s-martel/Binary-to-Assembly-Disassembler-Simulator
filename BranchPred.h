#ifndef __BRANCH_PRED_H
#define __BRANCH_PRED_H

#include <cstdint>
#include "Debug.h"
using namespace std;

#ifndef BPRED_SIZE
#define BPRED_SIZE 64
#endif

class BranchPred {
private:
	/* Your member variables here */
	struct PredictorEntry{
		int counter;
		uint32_t target;
	} predictor[BPRED_SIZE] = { {0} };
	int index;

	int predictions;
	int pred_takens;
	int mispredictions;
	int mispred_direction;
	int mispred_target;

public:
	BranchPred();

	/* Your member functions here */
	bool predict(uint32_t addr);
	void update(uint32_t addr, uint32_t target, bool predtaken, bool taken);
	bool showTaken(uint32_t addr);
	uint32_t showTarget(uint32_t addr);

	void printFinalStats();
};

#endif
