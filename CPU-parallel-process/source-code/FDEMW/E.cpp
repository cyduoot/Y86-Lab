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

int E_stat, E_icode, E_ifun, E_valC, E_valA, E_valB, E_dstE, E_dstM, E_srcA, E_srcB;
bool ZF, SF, OF, CF;

bool need_use_ALU;

int e_stat, e_icode, e_Cnd, e_valE, e_valA, e_dstE, e_dstM, e_aluA, e_aluB, e_alufun;

bool e_set_cc;

string E_op;

void ReadData(){
	scanf("%d%d%d%d%d%d%d%d%d%d", &E_stat, &E_icode, &E_ifun, &E_valC, &E_valA, &E_valB, &E_dstE, &E_dstM, &E_srcA, &E_srcB);
	int z, s, o, c;
	scanf("%d%d%d%d\n", &z, &s, &o, &c);
	ZF = z;
	SF = s;
	OF = o;
	CF = c;
}

void WriteData(){
	printf("%d %d %d %d %d %d %d %d %d %d\n", e_stat, e_icode, e_Cnd, e_valE, e_valA, e_dstE, e_dstM, e_aluA, e_aluB, e_alufun);
	int z, s, o, c;
	z = ZF;
	s = SF;
	o = OF;
	c = CF;
	printf("%d %d %d %d\n", z, s, o, c);
	cout << E_op << endl;
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

void setConditionCode(int a, int b, int t, int alufun, bool &ZF, bool &SF, bool &OF, bool &CF){
    ZF = (t == 0);
    SF = (t < 0);
    if (alufun == 0){
        OF = ((a < 0) == (b < 0)) && ((t < 0) != (a < 0));
        CF = (unsigned int) t < (unsigned int) a;
    }else if (alufun == 1){
        OF = ((a < 0) != (b < 0)) && ((t < 0) == (a < 0));
        CF = (unsigned int) t > (unsigned int) b;
    }else OF = CF = 0;
    E_op = E_op + "Renew Flag" + '\n';
}

void ALU(int alua, int alub, int alufun, int &valE, bool setCC, bool &ZF, bool &SF, bool &OF, bool &CF){
    if (alufun == 0) valE = alua + alub;
    else if (alufun == 1) valE = alub - alua;
    else if (alufun == 2) valE = alua & alub;
    else if (alufun == 3) valE = alua ^ alub;

    string s = "ALU : ";
    s = s + int2str(alua);
    if (alufun == 0) s = s + '+';
    else if (alufun == 1) s = s + '-';
    else if (alufun == 2) s = s + '&';
    else if (alufun == 3) s = s + '^';
    s = s + int2str(alub) + '=' + int2str(valE);
    E_op = E_op + s + '\n';

    if (setCC)
        setConditionCode(alua, alub, valE, alufun, ZF, SF, OF, CF);
}

void cond(int &e_Cnd, bool ZF, bool SF, bool OF, bool CF){
    if (E_icode == IRRMOVL || E_icode == IJXX){
        if (E_ifun == 0) e_Cnd = 1;
        if (E_ifun == 1) e_Cnd = (SF^OF) | ZF;
        if (E_ifun == 2) e_Cnd = (SF^OF);
        if (E_ifun == 3) e_Cnd = ZF;
        if (E_ifun == 4) e_Cnd = !ZF;
        if (E_ifun == 5) e_Cnd = !(SF^OF);
        if (E_ifun == 6) e_Cnd = !(SF^OF) & !ZF;
    }else e_Cnd = 0;
    E_op = E_op + "e_Cnd <- " + int2str(e_Cnd) + '\n';
}

void Execute(){
    E_op = "";
	
	need_use_ALU = 1;
    if (E_icode == IRRMOVL || E_icode == IOPL) e_aluA = E_valA;
    else if (E_icode == IIRMOVL || E_icode == IRMMOVL || E_icode == IMRMOVL) e_aluA = E_valC;
    else if (E_icode == ICALL || E_icode == IPUSHL) e_aluA = -4;
    else if (E_icode == IRET || E_icode == IPOPL) e_aluA = 4;
    else e_aluA = need_use_ALU = 0;

    if (E_icode == IRMMOVL || E_icode == IMRMOVL || E_icode == IOPL || E_icode == ICALL ||
        E_icode == IPUSHL || E_icode == IRET || E_icode == IPOPL) e_aluB = E_valB;
    else if (E_icode == IRRMOVL || E_icode == IIRMOVL) e_aluB = 0;
    else e_aluB = need_use_ALU = 0;

    if (E_icode == IOPL) e_alufun = E_ifun;
    else e_alufun = ALUADD;

/*
    e_set_cc = (E_icode == IOPL)
                && !(W_stat == SADR || W_stat == SINS || W_stat == SHLT)
                && !(M_stat == SADR || M_stat == SINS || M_stat == SHLT);
    if (e_set_cc){
        while (M->isRunning())
            Sleep(0);
        e_set_cc &= !(m_stat == SADR || m_stat == SINS || m_stat == SHLT);
    }
	*/
	e_set_cc = 1;

    if (need_use_ALU)
        ALU(e_aluA, e_aluB, e_alufun, e_valE, e_set_cc, ZF, SF, OF, CF);
    else e_valE = 0;

    cond(e_Cnd, ZF, SF, OF, CF);

    if (E_icode == IRRMOVL && !e_Cnd) e_dstE = RNONE;
    else e_dstE = E_dstE;
    if (e_dstE != RNONE)
        E_op = E_op + "e_dstE <- " + int2Reg(e_dstE) + '\n';

    e_icode = E_icode;
    e_stat = E_stat;
    e_valA = E_valA;
    e_dstM = E_dstM;
}

int main(){
	while (1){
		ReadData();
		Execute();
		WriteData();
	}
}