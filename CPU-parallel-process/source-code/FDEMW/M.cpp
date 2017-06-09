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

int M_stat, M_icode, M_Cnd, M_valE, M_valA, M_dstE, M_dstM;

int m_stat, m_icode, m_valE, m_valM, m_dstE, m_dstM;

int m_mem_addr, m_mem_data;

bool m_mem_read, m_mem_write, m_dimem_error;

string M_op;

void ReadData(){
	scanf("%d%d%d%d%d%d%d\n", &M_stat, &M_icode, &M_Cnd, &M_valE, &M_valA, &M_dstE, &M_dstM);
}

void WriteData(){
	printf("%d %d %d %d %d %d\n", m_stat, m_icode, m_valE, m_valM, m_dstE, m_dstM);
	cout << M_op << endl;
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

void mem_write(int head, int len, int data, bool &imem_error){
    printf("! %d %d %d\n", head, len, data);
	fflush(stdout);
	int t;
	scanf("%d", &t);
	imem_error = t;
}

void Memory(){
    if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL || M_icode == IMRMOVL) m_mem_addr = M_valE;
    else if (M_icode == IPOPL || M_icode == IRET) m_mem_addr = M_valA;
    else m_mem_addr = 0;

    if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL) m_mem_data = M_valA;
    else m_mem_data = 0;

    m_mem_read = (M_icode == IMRMOVL) || (M_icode == IPOPL) || (M_icode == IRET);
    m_mem_write = (M_icode == IRMMOVL) || (M_icode == IPUSHL) || (M_icode == ICALL);

    if (m_mem_read){
        mem_read(m_mem_addr, 4, m_valM, m_dimem_error);
        M_op = M_op + "m_valM <- M[" + int2str(m_mem_addr) + "] = " + int2str(m_valM) + '\n';
    }

    if (m_mem_write){
        mem_write(m_mem_addr, 4, m_mem_data, m_dimem_error);
        M_op = M_op + "M[" + int2str(m_mem_addr) + "] <- " + int2str(m_mem_data) + '\n';
    }

    if (m_dimem_error) m_stat = SADR;
    else m_stat = M_stat;

    m_icode = M_icode;
    m_valE = M_valE;
    m_dstE = M_dstE;
    m_dstM = M_dstM;
}

int main(){
	while (1){
		ReadData();
		Memory();
		WriteData();
	}
}