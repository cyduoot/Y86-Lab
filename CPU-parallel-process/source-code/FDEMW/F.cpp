#include <bits/stdc++.h>
using namespace std;

#define MAXLEN 100000

#define IHALT 0
#define INOP 1
#define IRRMOVL 2
#define IIRMOVL 3
#define IRMMOVL 4
#define IMRMOVL 5
#define IOPL 6
#define IJXX 7
#define ICALL 8
#define IRET 9
#define IPUSHL 10
#define IPOPL 11

#define FNONE 0

#define RESP 4
#define RNONE 15

#define ALUADD 0

#define SAOK 1
#define SADR 2
#define SINS 3
#define SHLT 4

#define SBUB 0

int F_predPC;
int M_icode, M_Cnd, M_valA;
int W_icode, W_valM;

int f_stat, f_icode, f_ifun, f_rA, f_rB, f_valC, f_valP, f_PC, f_predPC;

bool instr_valid, imem_error, need_regids, need_valC;

string F_op;

void ReadData(){
	scanf("%d%d%d%d%d%d\n", &F_predPC, &M_icode, &M_Cnd, &M_valA, &W_icode, &W_valM);
}

void WriteData(){
	printf("* %d %d %d %d %d %d %d %d %d\n", f_stat, f_icode, f_ifun, f_rA, f_rB, f_valC, f_valP, f_PC, f_predPC);
	cout << F_op << endl;
	fflush(stdout);
}

string int2str(int x){
    string s;
    stringstream ss;
    ss << x;
    s = ss.str();
    return s;
}

void mem_read(int head, int len, int &data, bool &imem_error){
	printf("? %d %d\n", head, len);
	fflush(stdout);
	int t;
	scanf("%d %d", &data, &t);
	imem_error = t;
}

void select_PC(){
    if (M_icode == IJXX && !M_Cnd){
        f_PC = M_valA;
        F_op = F_op + "f_pc <- M_valA = " + int2str(f_PC) + '\n';
    }
    else if (W_icode == IRET){
        f_PC = W_valM;
        F_op = F_op + "f_pc <- W_valM = " + int2str(f_PC) + '\n';
    }
    else{
        f_PC = F_predPC;
        F_op = F_op + "f_pc <- F_predPC = " + int2str(f_PC) + '\n';
    }
}

void Split(int f_PC, int &f_a, int &f_b, bool &imem_error){
    int x;
    f_a = f_b = 0;
    mem_read(f_PC, 1, x, imem_error);
    if (imem_error) return;

    f_a = x >> 4;
    f_b = x & 0xf;
}

void Fetch(){
	F_op = "";
	
    select_PC();

    Split(f_PC, f_icode, f_ifun, imem_error);
    f_PC++;

    instr_valid = !((f_icode != IRRMOVL && f_icode != IOPL && f_icode != IJXX && f_ifun != 0) || (f_icode > 11));

    need_regids = (f_icode == IRRMOVL || f_icode == IOPL || f_icode == IPUSHL || f_icode == IPOPL
         || f_icode == IIRMOVL || f_icode == IRMMOVL || f_icode == IMRMOVL);
    if (need_regids){
        Split(f_PC, f_rA, f_rB, imem_error);
        f_PC++;
    }else f_rA = f_rB = FNONE;

    need_valC = (f_icode == IIRMOVL || f_icode == IRMMOVL || f_icode == IMRMOVL || f_icode == IJXX || f_icode == ICALL);
    if (need_valC){
        mem_read(f_PC, 4, f_valC, imem_error);
        f_PC += 4;
    }else f_valC = 0;

    f_valP = f_PC;

    if (imem_error) f_stat = SADR;
    else if (!instr_valid) f_stat = SINS;
    else if (f_icode == IHALT) f_stat = SHLT;
    else f_stat = SAOK;

    if (f_icode == IJXX || f_icode == ICALL) f_predPC = f_valC;
    else f_predPC = f_valP;
    F_op = F_op + "f_predPC <- " + int2str(f_predPC) + '\n';
}

int main(){
	while (1){
		ReadData();
		Fetch();
		WriteData();
	}
}
