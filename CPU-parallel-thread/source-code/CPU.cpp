#include "CPU.h"

CPU *CPU_Target;

void Process_F :: run(){
    CPU_Target -> Fetch();
}

void Process_D :: run(){
    CPU_Target -> Decode();
}

void Process_E :: run(){
    CPU_Target -> Execute();
}

void Process_M :: run(){
    CPU_Target -> Memory();
}

void Process_W :: run(){
    CPU_Target -> Write();
}

string CPU :: int2str(int x){
    string s;
    stringstream ss;
    ss << x;
    s = ss.str();
    return s;
}

string CPU :: int2Reg(int x){
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

void CPU :: prepare(){
    F_op = "";
    D_op = "";
    E_op = "";
    M_op = "";
    W_op = "";

    memset(this, 0, sizeof(*this));
    F_predPC = code_head;
    D_icode = E_icode = M_icode = 1;
    D_stat = E_stat = M_stat = Stat = 1;
    D_rA = D_rB = RNONE;
    E_dstE = E_dstM = E_srcA = E_srcB = RNONE;
    M_dstE = M_dstM = RNONE;
    W_dstE = W_dstM = RNONE;
    f_predPC = code_head;
    f_icode = d_icode = e_icode = m_icode = 1;
    f_stat = d_stat = e_stat = m_stat = Stat = 1;
    f_rA = f_rB = RNONE;
    d_dstE = d_dstM = d_srcA = d_srcB = RNONE;
    e_dstE = e_dstM = RNONE;
    m_dstE = m_dstM = RNONE;
}

void CPU :: mem_read(int head, int len, int &data, bool &imem_error){
    data = 0;
    for (int i = 0; i < len; i++)
        if (head + i >= 0){
            int x = mem[head + i];
            x = x & (0xff);
            data =  data ^ (x << (i * 8));
        }else{
            imem_error = 1;
            break;
        }
}

void CPU :: mem_write(int head, int len, int data, bool &imem_error){
    for (int i = 0; i < len; i++)
        if (head + i >= 0){
            mem[head + i] = data & (0xff);
            data >>= 8;
        }else{
            imem_error = 1;
            break;
        }
}

void CPU :: read_in(QString path){
    string st = path.toStdString();
    const char *pch = st.c_str();
    FILE *bin_stream;
    int head = code_head;
    bin_stream = fopen(pch, "r");
    len = fread(bin_code, sizeof(char), MAXLEN, bin_stream);
    for (int i = 0; i < len; i++){
        mem_write(head, 1, bin_code[i], imem_error);
        head++;
    }
}

void CPU :: select_PC(){
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

void CPU :: Split(int f_PC, int &f_a, int &f_b, bool &imem_error){
    int x;
    f_a = f_b = 0;
    mem_read(f_PC, 1, x, imem_error);
    if (imem_error) return;

    f_a = x >> 4;
    f_b = x & 0xf;
}

void CPU :: SelFwdA(){
    if (D_icode == ICALL || D_icode == IJXX) d_valA = D_valP;
    else{
        while (E->isRunning())
            Sleep(0);
        if (d_srcA == e_dstE)
            d_valA = e_valE;
        else if (d_srcA == M_dstM){
            while (M->isRunning())
                Sleep(0);
            d_valA = m_valM;
        }
        else if (d_srcA == M_dstE) d_valA = M_valE;
        else if (d_srcA == W_dstM) d_valA = W_valM;
        else if (d_srcA == W_dstE) d_valA = W_valE;
        else d_valA = d_rvalA;
    }
    D_op = D_op + "d_valA <- " + int2str(d_valA) + '\n';
}

void CPU :: FwdB(){
    while (E->isRunning())
        Sleep(0);
    if (d_srcB == e_dstE)
        d_valB = e_valE;
    else if (d_srcB == M_dstM){
        while (M->isRunning())
            Sleep(0);
        d_valB = m_valM;
    }
    else if (d_srcB == M_dstE) d_valB = M_valE;
    else if (d_srcB == W_dstM) d_valB = W_valM;
    else if (d_srcB == W_dstE) d_valB = W_valE;
    else d_valB = d_rvalB;
    D_op = D_op + "d_valB <- " + int2str(d_valB) + '\n';
}

int CPU :: get_Register(int src){
    if (src < 8) return Reg[src];
    return 0xf;
}

void CPU :: set_Register(int src, int val){
    if (src < 8){
        Reg[src] = val;
        W_op = W_op + "Reg[" + int2Reg(src) + "] <- " + int2str(val) + '\n';
    }
}

void CPU :: setConditionCode(int a, int b, int t, int alufun, bool &ZF, bool &SF, bool &OF, bool &CF){
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

void CPU :: ALU(int alua, int alub, int alufun, int &valE, bool setCC, bool &ZF, bool &SF, bool &OF, bool &CF){
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

void CPU :: cond(int &e_Cnd, bool ZF, bool SF, bool OF, bool CF){
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

void CPU :: F_Control(){
    F_stall = (E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB);
    F_stall |= (D_icode == IRET || E_icode == IRET || M_icode == IRET);
}

void CPU :: D_Control(){
    D_stall = (E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB);
    D_bubble = (E_icode == IJXX && !e_Cnd);
    D_bubble |= (!((E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB))) && (IRET == D_icode || IRET == E_icode || IRET == M_icode);
}

void CPU :: E_Control(){
    E_bubble = (E_icode == IJXX && !e_Cnd);
    E_bubble |= (E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB);
}

void CPU :: M_Control(){
    M_bubble = (m_stat == SADR || m_stat == SINS || m_stat == SHLT);
    M_bubble |= (W_stat == SADR || W_stat == SINS || W_stat == SHLT);
}

void CPU :: W_Control(){
    W_stall = (W_stat == SADR || W_stat == SINS || W_stat == SHLT);
}

void CPU :: Control(){
    F_Control();
    D_Control();
    E_Control();
    M_Control();
    W_Control();
}

void CPU :: Send(){
    F_op = "";
    D_op = "";
    E_op = "";
    M_op = "";
    W_op = "";

    if (!F_stall && !F_bubble)
        F_predPC = f_predPC;

    if (!D_stall && !D_bubble){
        D_stat = f_stat;
        D_icode = f_icode;
        D_ifun = f_ifun;
        D_rA = f_rA;
        D_rB = f_rB;
        D_valC = f_valC;
        D_valP = f_valP;
    }

    if (!D_stall && D_bubble){
        D_stat = SBUB;
        D_icode = 1;
        D_ifun = 0;
        D_rA = D_rB = RNONE;
        D_valC = D_valP = 0;
    }

    if (!E_stall && !E_bubble){
        E_stat = d_stat;
        E_icode = d_icode;
        E_ifun = d_ifun;
        E_valC = d_valC;
        E_valA = d_valA;
        E_valB = d_valB;
        E_dstE = d_dstE;
        E_dstM = d_dstM;
        E_srcA = d_srcA;
        E_srcB = d_srcB;
    }

    if (!E_stall && E_bubble){
        E_stat = SBUB;
        E_icode = 1;
        E_ifun = 0;
        E_valA = E_valB = 0;
        E_dstE = E_dstM = RNONE;
        E_srcA = E_srcB = RNONE;
    }

    if (!M_stall && !M_bubble){
        M_stat = e_stat;
        M_icode = e_icode;
        M_Cnd = e_Cnd;
        M_valE = e_valE;
        M_valA = e_valA;
        M_dstE = e_dstE;
        M_dstM = e_dstM;
    }

    if (!M_stall && M_bubble){
        M_stat = SBUB;
        M_icode = 1;
        M_Cnd = 0;
        M_valE = M_valA = 0;
        M_dstE = M_dstM = RNONE;
    }

    if (!W_stall && !W_bubble){
        W_stat = m_stat;
        W_icode = m_icode;
        W_valE = m_valE;
        W_valM = m_valM;
        W_dstE = m_dstE;
        W_dstM = m_dstM;
    }
}

void CPU :: Fetch(){
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

void CPU :: Decode(){
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

void CPU :: Execute(){
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

    e_set_cc = (E_icode == IOPL)
                && !(W_stat == SADR || W_stat == SINS || W_stat == SHLT)
                && !(M_stat == SADR || M_stat == SINS || M_stat == SHLT);

    if (e_set_cc){
        while (M->isRunning())
            Sleep(0);
        e_set_cc &= !(m_stat == SADR || m_stat == SINS || m_stat == SHLT);
    }

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

void CPU :: Memory(){
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

void CPU :: Write(){
    if (W_stat == SBUB) Stat = SAOK;
    else Stat = W_stat;

    if (Stat != SAOK) return;

    set_Register(W_dstE, W_valE);
    set_Register(W_dstM, W_valM);
}

void CPU :: some_prepare_work(){

}

void CPU :: FFF(){
    CPU_Target = this;
    F = new Process_F();
    D = new Process_D();
    E = new Process_E();
    M = new Process_M();
    W = new Process_W();
    W -> start();
    M -> start();
    E -> start();
    D -> start();
    F -> start();
    while (W->isRunning())
        Sleep(0);
    while (M->isRunning())
        Sleep(0);
    while (E->isRunning())
        Sleep(0);
    while (D->isRunning())
        Sleep(0);
    while (F->isRunning())
        Sleep(0);
    delete F;
    delete D;
    delete E;
    delete M;
    delete W;
    qDebug() << "done";
}
