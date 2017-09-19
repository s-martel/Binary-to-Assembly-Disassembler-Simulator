/****************************
* Steve Martel
* CS 3339 Spring 2016
****************************/
#include "CPU.h"

const string CPU::regNames[] = { "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
"$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra" };


CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
	for (int i = 0; i < NREGS; i++) {
		regFile[i] = 0;
	}
	hi = 0;
	lo = 0;
	regFile[28] = 0x10008000; // gp
	regFile[29] = 0x10000000 + dMem.getSize(); // sp

	instructions = 0;
	stop = false;
}

int sllO, sraO, jrO, mfhiO, mfloO, multO, divO, adduO, subuO, sltO, jO, jalO, beqO, bneO, addiO, addiuO, andiO, luiO, trapO, lwO, swO = 0;

void CPU::run() {
	while (!stop) {
		instructions++;

		fetch();
		decode();
		execute();

		D(printRegFile());

		stat.clock(IF1);
	}
}

void CPU::fetch() {
	instr = iMem.loadWord(pc);
	pc = pc + 4;
}

void CPU::decode() {
	uint32_t opcode = instr >> 26;
	uint32_t rs = (instr >> 21) & 0x1f;
	uint32_t rt = (instr >> 16) & 0x1f;
	uint32_t rd = (instr >> 11) & 0x1f;
	uint32_t shamt = (instr >> 6) & 0x1f;
	uint32_t funct = instr & 0x3f;
	uint32_t uimm = instr & 0xffff;
	int32_t  simm = ((signed)uimm << 16) >> 16;
	uint32_t addr = instr & 0x3ffffff;

	opIsLoad = false;
	opIsStore = false;
	opIsMultDiv = false;
	writeDest = false;
	aluOp = OUT_S1; // default value to prevent garbage input to ALU

	uint32_t branch = pc;
	bool predTaken;

	D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
	switch (opcode) {
	case 0x00:
		switch (funct) {
		case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
			aluOp = SHF_L;          // Direct usage of SHF_L in ALU
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = shamt;
			break;
		case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
			aluOp = SHF_R;          // Direct usage of SHF_R in ALU
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = shamt;
			break;
		case 0x08: D(cout << "jr " << regNames[rs]);
			// No ALU output needed, pc set to  value in $ra
			pc = regFile[rs]; stat.registerSrc(rs, ID);
			stat.flush(ID - IF1);
			break;
		case 0x10: D(cout << "mfhi " << regNames[rd]);
			aluOp = OUT_S1;
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = hi; stat.registerSrc(REG_HILO, EXE1);
			break;
		case 0x12: D(cout << "mflo " << regNames[rd]);
			aluOp = OUT_S1;
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = lo; stat.registerSrc(REG_HILO, EXE1);
			break;
		case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
			aluOp = MUL;            // Direct usage of MULT in ALU
			opIsMultDiv = true; stat.registerDest(REG_HILO, WB);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = regFile[rt]; stat.registerSrc(rt, EXE1);
			break;
		case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
			aluOp = DIV;            // Direct usage of DIV in ALU
			opIsMultDiv = true; stat.registerDest(REG_HILO, WB);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = regFile[rt]; stat.registerSrc(rt, EXE1);
			break;
		case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
			aluOp = ADD;            // Direct usage of ADD in ALU (without overflow trapping)
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = regFile[rt]; stat.registerSrc(rt, EXE1);
			break;
		case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
			aluOp = ADD;            // Direct usage of ADD in ALU (without overflow trapping)
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = -(regFile[rt]); stat.registerSrc(rt, EXE1);
			break;
		case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
			aluOp = CMP_LT;         // Direct usage of CMP_LT in ALU
			writeDest = true; destReg = rd; stat.registerDest(rd, MEM1);
			aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
			aluSrc2 = regFile[rt]; stat.registerSrc(rt, EXE1);
			break;
		default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
		}
		break;
	case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
		pc = (pc & 0xf0000000) | addr << 2;
		stat.flush(ID - IF1);
		break;
	case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
		aluOp = OUT_S1; // pass src1 straight through ALU
		writeDest = true; destReg = REG_RA; stat.registerDest(REG_RA, EXE1);
		aluSrc1 = pc;
		pc = (pc & 0xf0000000) | addr << 2;
		stat.flush(ID - IF1);
		break;
	case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
		stat.registerSrc(rs, ID); stat.registerSrc(rt, ID);
		stat.countBranch();

		predTaken = branchpred.predict(branch);  // predict branch taken/not taken
		if (regFile[rs] == regFile[rt]) {
			pc = (pc + (simm << 2));
			stat.countTaken();
			if (!branchpred.showTaken(branch) || branchpred.showTarget(branch) != pc) stat.flush(ID - IF1);
		}
		branchpred.update(branch, pc, predTaken, regFile[rs] == regFile[rt]);  // update stored branch data
		break;
	case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
		stat.registerSrc(rs, ID); stat.registerSrc(rt, ID);
		stat.countBranch();
				
		predTaken = branchpred.predict(branch);  // predict branch taken/not taken
		if (regFile[rs] != regFile[rt]) {
			pc = (pc + (simm << 2));
			stat.countTaken();
			if (!branchpred.showTaken(branch) || branchpred.showTarget(branch) != pc) stat.flush(ID - IF1);
		}
		branchpred.update(branch, pc, predTaken, regFile[rs] != regFile[rt]);  // update stored branch data
		break;
	case 0x08: D(cout << "addi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
		aluOp = ADD;               // Direct use of ADD in ALU
		writeDest = true; destReg = rt; stat.registerDest(rt, MEM1);
		aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
		aluSrc2 = simm;
		break;
	case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
		aluOp = ADD;               // Direct use of ADD in ALU
		writeDest = true; destReg = rt; stat.registerDest(rt, MEM1);
		aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
		aluSrc2 = simm;
		break;
	case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
		aluOp = AND;              // Direct use of AND in ALU
		writeDest = true; destReg = rt; stat.registerDest(rt, MEM1);
		aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
		aluSrc2 = uimm;
		break;
	case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
		aluOp = SHF_L;            // Use SHF_L to capture upper 16 bits
		writeDest = true; destReg = rt; stat.registerDest(rt, MEM1);
		aluSrc1 = simm;
		aluSrc2 = 16;
		break;
	case 0x1a: D(cout << "trap " << hex << addr);
		aluOp = OUT_S1; // don't need the ALU
		switch (addr & 0xf) {
		case 0x0: cout << endl; break;
		case 0x1: cout << " " << (signed)regFile[rs]; stat.registerSrc(rs, EXE1);
			break;
		case 0x5: cout << endl << "? "; cin >> regFile[rt]; stat.registerDest(rt, MEM1);
			break;
		case 0xa: stop = true; break;
		default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
			stop = true;
		}
		break;
	case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
		opIsLoad = true;
		aluOp = ADD;              // Use ADD in ALU to generate data address from offset
		writeDest = true; destReg = rt; stat.registerDest(rt, WB);
		aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
		aluSrc2 = simm;
		stat.countMemOp();
		break;
	case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
		opIsStore = true;
		aluOp = ADD;              // Use ADD in ALU to generate data address from offset
		aluSrc1 = regFile[rs]; stat.registerSrc(rs, EXE1);
		aluSrc2 = simm;
		storeData = regFile[rt]; stat.registerSrc(rt, MEM1);
		stat.countMemOp();
		break;
	default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
	}
	D(cout << endl);
}

void CPU::execute() {
	uint32_t aluOut = alu.op(aluOp, aluSrc1, aluSrc2);

	if (opIsLoad) {
		stat.stall(cachestat.access(aluOut, LOAD));  // cache/memory stall
		aluOut = dMem.loadWord(aluOut);
	}
	else if (opIsStore) {
		stat.stall(cachestat.access(aluOut, STORE));  // cache/memory stall
		dMem.storeWord(storeData, aluOut);
	}
	else if (opIsMultDiv) {
		hi = alu.getUpper();
		lo = alu.getLower();
	}

	// Regfile update (but never write to register 0)
	if (writeDest && destReg > 0)
		regFile[destReg] = aluOut;
}

void CPU::printRegFile() {
	cout << hex;
	for (int i = 0; i < NREGS; i++) {
		cout << "    " << regNames[i];
		if (i > 0) cout << "  ";
		cout << ": " << setfill('0') << setw(8) << regFile[i];
		if (i == (NREGS - 1) || (i + 1) % 4 == 0)
			cout << endl;
	}
	cout << "    hi   : " << setfill('0') << setw(8) << hi;
	cout << "    lo   : " << setfill('0') << setw(8) << lo;
	cout << dec << endl;
}

void CPU::printFinalStats() {
	cout << "Program finished at pc = 0x" << hex << pc << "  ("
		<< dec << instructions << " instructions executed)" << endl << endl;

	cout << "Cycles: " << stat.getCycles() << endl
		<< "CPI: " << fixed << setprecision(1) << (float)stat.getCycles() / instructions << endl << endl;

	cout << "Bubbles: " << stat.getBubbles() << endl
		<< "Flushes: " << stat.getFlushes() << endl
		<< "Stalls: " << stat.getStalls() << endl << endl;

	branchpred.printFinalStats();

// 	cachestat.printFinalStats();

	// 	int totalraws = stat.getRAWhazards(PIPESTAGES);
	// 	cout << "RAW hazards: " << totalraws << " (1 per every " << fixed << setprecision(2) << 1.0 * instructions / totalraws << " instructions)" << endl;
	// 	cout << setprecision(0);
	// 	cout << "  On EXE1 op: " << stat.getRAWhazards(EXE1) << " (" << 100.0 * stat.getRAWhazards(EXE1) / totalraws << "%)" << endl
	// 		<< "  On EXE2 op: " << stat.getRAWhazards(EXE2) << " (" << 100.0 * stat.getRAWhazards(EXE2) / totalraws << "%)" << endl
	// 		<< "  On MEM1 op: " << stat.getRAWhazards(MEM1) << " (" << 100.0 * stat.getRAWhazards(MEM1) / totalraws << "%)" << endl
	// 		<< "  On MEM2 op: " << stat.getRAWhazards(MEM2) << " (" << 100.0 * stat.getRAWhazards(MEM2) / totalraws << "%)" << endl;
}
