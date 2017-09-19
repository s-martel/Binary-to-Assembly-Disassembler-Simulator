#include <iostream>
#include <iomanip>
#include <cstdint>
#include "BranchPred.h"
using namespace std;

BranchPred::BranchPred() {
	cout << "Branch Predictor Entries: " << BPRED_SIZE << endl;

	/* You should have some code that goes here */
	PredictorEntry predictor[BPRED_SIZE];
	for (int i = 0; i < BPRED_SIZE; ++i) {
		predictor[i].counter = 0;
		predictor[i].target = -1;
	}
	
	predictions = 0;
	pred_takens = 0;
	mispredictions = 0;
	mispred_direction = 0;
	mispred_target = 0;
}

/* You should add functions here */
bool BranchPred::predict(uint32_t addr) {
	index = (addr >> 2) % BPRED_SIZE;
	++predictions;
	if (predictor[index].counter > 1) {
		++pred_takens;
		return true;  // return if 2-bit counter indicates branch taken
	}
	else return false;
}

void BranchPred::update(uint32_t addr, uint32_t target, bool predtaken, bool taken) {
	index = (addr >> 2) % BPRED_SIZE;

	if (taken) {
		if (predictor[index].counter < 3) ++predictor[index].counter;
		if (predtaken) {
			if (predictor[index].target != target) {
				++mispred_target;
				++mispredictions;
				predictor[index].target = target;
				return;  // Branch taken, predicted taken, wrong target
			}
			return;  // Branch taken, predicted taken, correct target
		}
		++mispred_direction;
		++mispredictions;
		predictor[index].target = target;
		return;  // Branch taken, predicted not taken
	}

	if (!taken) {
		if (predictor[index].counter > 0) --predictor[index].counter;
		if (predtaken) {
			++mispred_direction;
			++mispredictions;
			return;  // Branch not taken, predicted taken
		}
		return;  // Branch not taken, predicted not taken
	}
}

bool BranchPred::showTaken(uint32_t addr) {
	index = (addr >> 2) % BPRED_SIZE;
	if (predictor[index].counter > 1) return true;  // Counter indicates branch taken
	else return false;
}

uint32_t BranchPred::showTarget(uint32_t addr) {
	index = (addr >> 2) % BPRED_SIZE;
	return predictor[index].target;  // Stored branch target
}

void BranchPred::printFinalStats() {
	int correct = predictions - mispredictions;
	int not_takens = predictions - pred_takens;

	cout << setprecision(1);
	cout << "Branches predicted: " << predictions << endl;
	cout << "  Pred T: " << pred_takens << " ("
		<< (100.0 * pred_takens / predictions) << "%)" << endl;
	cout << "  Pred NT: " << not_takens << endl;
	cout << "Mispredictions: " << mispredictions << " ("
		<< (100.0 * mispredictions / predictions) << "%)" << endl;
	cout << "  Mispredicted direction: " << mispred_direction << endl;
	cout << "  Mispredicted target: " << mispred_target << endl;
	cout << "Predictor accuracy: " << (100.0 * correct / predictions) << "%" << endl;
}
