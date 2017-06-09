#ifndef CPU_H
#define CPU_H

#include <bits/stdc++.h>
#include <QString>
#include <QThread>
#include "global.h"
#include <windows.h>
#include <QDebug>
using namespace std;

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

class CPU;

class CPU{
    public:

    char bin_code[MAXLEN];
    int circle_time, speed;
    bool running;

    int Reg[16];
    int Stat;
    bool ZF, OF, SF, CF;
    bool instr_valid, imem_error, need_regids, need_valC;

    int F_predPC;
    int f_stat, f_icode, f_ifun, f_rA, f_rB, f_valC, f_valP, f_PC, f_predPC;
    bool F_stall, F_bubble;

    int D_stat, D_icode, D_ifun, D_rA, D_rB, D_valC, D_valP;
    int d_stat, d_icode, d_ifun, d_valC, d_valA, d_valB, d_dstE, d_dstM, d_srcA, d_srcB, d_rvalA, d_rvalB;
    bool D_stall, D_bubble;

    int E_stat, E_icode, E_ifun, E_valC, E_valA, E_valB, E_dstE, E_dstM, E_srcA, E_srcB;
    int e_stat, e_icode, e_Cnd, e_valE, e_valA, e_dstE, e_dstM, e_aluA, e_aluB, e_alufun;
    bool e_set_cc;
    bool E_stall, E_bubble;

    bool need_use_ALU;
    int M_stat, M_icode, M_Cnd, M_valE, M_valA, M_dstE, M_dstM;
    int m_stat, m_icode, m_valE, m_valM, m_dstE, m_dstM, m_mem_addr, m_mem_data;
    bool m_mem_read, m_mem_write, m_dimem_error;
    bool M_stall, M_bubble;

    int W_stat, W_icode, W_valE, W_valM, W_dstE, W_dstM;
    bool W_stall, W_bubble;

    void prepare();

    void mem_read(int head, int len, int &data, bool &imem_error);

    void mem_write(int head, int len, int data, bool &imem_error);

    void read_in(QString path);

    void F_Control();

    void D_Control();

    void E_Control();

    void M_Control();

    void W_Control();

    void Control();

    void Send();

    void FFF();

    QProcess &F, &D, &E, &M, &W;

    signals:
    public slots:
    void F_ret();
    void D_ret();
    void E_ret();
    void M_ret();
    void W_ret();
};

#endif

