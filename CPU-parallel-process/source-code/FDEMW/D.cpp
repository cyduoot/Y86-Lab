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

int D_stat, D_icode, D_ifun, D_rA, D_rB, D_valC, D_valP;

int M_dstM, M_dstE, M_valE, W_dstM, W_valM, W_dstE, W_valE;

int d_stat, d_icode, d_ifun, d_valC, d_valA, d_valB, d_dstE, d_dstM, d_srcA, d_srcB, d_rvalA, d_rvalB;

int marked_A_e, marked_A_m, marked_B_e, marked_B_m;

string D_op;

void ReadData(){
	scanf("%d%d%d%d%d%d%d", &D_stat, &D_icode, &D_ifun, &D_rA, &D_rB, &D_valC, &D_valP);
	scanf("%d%d%d%d%d%d%d", &M_dstM, &M_dstE, &M_valE, &W_dstM, &W_valM, &W_dstE, &W_valE);
}

void WriteData(){
	printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", d_stat, d_icode, d_ifun, d_valC, d_valA, d_valB, d_dstE, d_dstM, d_srcA, d_srcB, d_rvalA, d_rvalB);
	printf("%d %d %d %d\n", marked_A_e, marked_A_m, marked_B_e, marked_B_m);
	cout << D_op << endl;
	fflush(stdout);
}

string int2str(int x){
    string s;
    stringstream ss;
    ss << x;
    s = ss.str();
    return s;
}

string int2Reg(int x){
    switch (x){
        case 0:return "%eax";
        case 1:return "%ecx";
        case 2:return "%edx";
        case 3:return "%ebx";
        case 4:return "%esp";
        case 5:return "%ebp";
        case 6:return "%esi";
        case 7:return "%edi";
    }
    return "RNONE";
}

int get_Register(int src){
    if (src < 8){
		printf("? %d\n", src);
		fflush(stdout);
		int x;
		scanf("%d", &x);
		return x;
	}
	else return 0xf;
}

void SelFwdA(){
	marked_A_e = 0;
	marked_A_m = 0;
    if (D_icode == ICALL || D_icode == IJXX) d_valA = D_valP;
    else{
		marked_A_e = 1;
		d_valA = 0;
        if (d_srcA == M_dstM){
			marked_A_m = 1;
            d_valA = 0;
        }
        else if (d_srcA == M_dstE) d_valA = M_valE;
        else if (d_srcA == W_dstM) d_valA = W_valM;
        else if (d_srcA == W_dstE) d_valA = W_valE;
        else d_valA = d_rvalA;
    }
    D_op = D_op + "d_valA <- " + int2str(d_valA) + '\n';
}

void FwdB(){
	marked_B_e = 1;
	marked_B_m = 0;
    if (d_srcB == M_dstM){
		marked_B_m = 1;
        d_valB = 0;
    }
    else if (d_srcB == M_dstE) d_valB = M_valE;
    else if (d_srcB == W_dstM) d_valB = W_valM;
    else if (d_srcB == W_dstE) d_valB = W_valE;
    else d_valB = d_rvalB;
    D_op = D_op + "d_valB <- " + int2str(d_valB) + '\n';
}

void Decode(){
	D_op = "";
	
    if (D_icode == IRRMOVL || D_icode == IRMMOVL || D_icode == IOPL || D_icode == IPUSHL) d_srcA = D_rA;
    else if (D_icode == IPOPL || D_icode == IRET) d_srcA = RESP;
    else d_srcA = RNONE;
    if (d_srcA != RNONE)
        D_op = D_op + "d_srcA <- " + int2Reg(d_srcA) + '\n';

    if (D_icode == IOPL || D_icode == IRMMOVL || D_icode == IMRMOVL) d_srcB = D_rB;
    else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_srcB = RESP;
    else d_srcB = RNONE;
    if (d_srcB != RNONE)
        D_op = D_op + "d_srcB <- " + int2Reg(d_srcB) + '\n';

    d_rvalA = get_Register(d_srcA);

    d_rvalB = get_Register(d_srcB);

    if (D_icode == IRRMOVL || D_icode == IIRMOVL || D_icode == IOPL) d_dstE = D_rB;
    else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_dstE = RESP;
    else d_dstE = RNONE;
    if (d_dstE != RNONE)
        D_op = D_op + "d_dstE <- " + int2Reg(d_dstE) + '\n';

    if (D_icode == IMRMOVL || D_icode == IPOPL) d_dstM = D_rA;
    else d_dstM = RNONE;
    if (d_dstM != RNONE)
        D_op = D_op + "d_dstM <- " + int2Reg(d_dstM) + '\n';

    SelFwdA();

    FwdB();

    d_icode = D_icode;
    d_ifun = D_ifun;
    d_stat = D_stat;
    d_valC = D_valC;
}

int main(){
	while (1){
		ReadData();
		Decode();
		WriteData();
	}
}