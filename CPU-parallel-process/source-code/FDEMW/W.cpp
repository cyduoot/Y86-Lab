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

int W_stat, W_icode, W_valE, W_valM, W_dstE, W_dstM;

int Stat;

string W_op;

void ReadData(){
	scanf("%d%d%d%d%d%d\n", &W_stat, &W_icode, &W_valE, &W_valM, &W_dstE, &W_dstM);
}

void WriteData(){
	printf("* %d\n", Stat);
	cout << W_op << endl;
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

void set_Register(int src, int val){
    if (src < 8){
		printf("! %d %d\n", src, val);
        W_op = W_op + "Reg[" + int2Reg(src) + "] <- " + int2str(val) + '\n';
    }
}

void Write(){
    if (W_stat == SBUB) Stat = SAOK;
    else Stat = W_stat;

    if (Stat != SAOK) return;

    set_Register(W_dstE, W_valE);
    set_Register(W_dstM, W_valM);
}

int main(){
	while (1){
		ReadData();
		Write();
		WriteData();
	}
}
