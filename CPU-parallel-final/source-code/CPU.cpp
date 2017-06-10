#include "CPU.h"
#include "simulator.h"
#include <sstream>

CPU *CPU_Target;

void Process_F :: run(){
    CPU_Target -> Fetch_thread();
}

void Process_D :: run(){
    CPU_Target -> Decode_thread();
}

void Process_E :: run(){
    CPU_Target -> Execute_thread();
}

void Process_M :: run(){
    CPU_Target -> Memory_thread();
}

void Process_W :: run(){
    CPU_Target -> Write_thread();
}

void CPU :: prepare(){
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
    std :: string st = path.toStdString();
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
    }
    else if (W_icode == IRET){
        f_PC = W_valM;
    }
    else{
        f_PC = F_predPC;
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

void CPU :: SelFwdA_serial(){
    if (D_icode == ICALL || D_icode == IJXX) d_valA = D_valP;
    else if (d_srcA == e_dstE) d_valA = e_valE;
    else if (d_srcA == M_dstM) d_valA = m_valM;
    else if (d_srcA == M_dstE) d_valA = M_valE;
    else if (d_srcA == W_dstM) d_valA = W_valM;
    else if (d_srcA == W_dstE) d_valA = W_valE;
    else d_valA = d_rvalA;
}

void CPU :: FwdB_serial(){
    if (d_srcB == e_dstE) d_valB = e_valE;
    else if (d_srcB == M_dstM) d_valB = m_valM;
    else if (d_srcB == M_dstE) d_valB = M_valE;
    else if (d_srcB == W_dstM) d_valB = W_valM;
    else if (d_srcB == W_dstE) d_valB = W_valE;
    else d_valB = d_rvalB;
}

void CPU :: SelFwdA_thread(){
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
}

void CPU :: FwdB_thread(){
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
}

int CPU :: get_Register(int src){
    if (src < 8) return Reg[src];
    return 0xf;
}

bool CPU :: set_Register(int src, int val){
    if (src < 8){
        Reg[src] = val;
        return 1;
    }
    return 0;
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
}

void CPU :: ALU(int alua, int alub, int alufun, int &valE, bool setCC, bool &ZF, bool &SF, bool &OF, bool &CF){
    if (alufun == 0) valE = alua + alub;
    else if (alufun == 1) valE = alub - alua;
    else if (alufun == 2) valE = alua & alub;
    else if (alufun == 3) valE = alua ^ alub;

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


void CPU :: Fetch_serial(){
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
}

void CPU :: Decode_serial(){
    if (D_icode == IRRMOVL || D_icode == IRMMOVL || D_icode == IOPL || D_icode == IPUSHL) d_srcA = D_rA;
    else if (D_icode == IPOPL || D_icode == IRET) d_srcA = RESP;
    else d_srcA = RNONE;

    if (D_icode == IOPL || D_icode == IRMMOVL || D_icode == IMRMOVL) d_srcB = D_rB;
    else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_srcB = RESP;
    else d_srcB = RNONE;

    d_rvalA = get_Register(d_srcA);

    d_rvalB = get_Register(d_srcB);

    if (D_icode == IRRMOVL || D_icode == IIRMOVL || D_icode == IOPL) d_dstE = D_rB;
    else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_dstE = RESP;
    else d_dstE = RNONE;

    if (D_icode == IMRMOVL || D_icode == IPOPL) d_dstM = D_rA;
    else d_dstM = RNONE;

    SelFwdA_serial();

    FwdB_serial();

    d_icode = D_icode;
    d_ifun = D_ifun;
    d_stat = D_stat;
    d_valC = D_valC;
}

void CPU :: Execute_serial(){
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

    e_set_cc = (E_icode == IOPL) &&
                !(m_stat == SADR || m_stat == SINS || m_stat == SHLT) &&
                !(W_stat == SADR || W_stat == SINS || W_stat == SHLT);

    if (need_use_ALU)
        ALU(e_aluA, e_aluB, e_alufun, e_valE, e_set_cc, ZF, SF, OF, CF);
    else e_valE = 0;

    cond(e_Cnd, ZF, SF, OF, CF);

    if (E_icode == IRRMOVL && !e_Cnd) e_dstE = RNONE;
    else e_dstE = E_dstE;

    e_icode = E_icode;
    e_stat = E_stat;
    e_valA = E_valA;
    e_dstM = E_dstM;
}

void CPU :: Memory_serial(){
    if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL || M_icode == IMRMOVL) m_mem_addr = M_valE;
    else if (M_icode == IPOPL || M_icode == IRET) m_mem_addr = M_valA;
    else m_mem_addr = 0;

    if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL) m_mem_data = M_valA;
    else m_mem_data = 0;

    m_mem_read = (M_icode == IMRMOVL) || (M_icode == IPOPL) || (M_icode == IRET);
    m_mem_write = (M_icode == IRMMOVL) || (M_icode == IPUSHL) || (M_icode == ICALL);

    if (m_mem_read){
        mem_read(m_mem_addr, 4, m_valM, m_dimem_error);
    }

    if (m_mem_write){
        mem_write(m_mem_addr, 4, m_mem_data, m_dimem_error);
    }

    if (m_dimem_error) m_stat = SADR;
    else m_stat = M_stat;

    m_icode = M_icode;
    m_valE = M_valE;
    m_dstE = M_dstE;
    m_dstM = M_dstM;
}

void CPU :: Write_serial(){
    if (W_stat == SBUB) Stat = SAOK;
    else Stat = W_stat;

    if (Stat != SAOK) return;

    set_Register(W_dstE, W_valE);
    set_Register(W_dstM, W_valM);
}

void CPU :: Fetch_thread(){
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
}

void CPU :: Decode_thread(){
    if (D_icode == IRRMOVL || D_icode == IRMMOVL || D_icode == IOPL || D_icode == IPUSHL) d_srcA = D_rA;
    else if (D_icode == IPOPL || D_icode == IRET) d_srcA = RESP;
    else d_srcA = RNONE;

    if (D_icode == IOPL || D_icode == IRMMOVL || D_icode == IMRMOVL) d_srcB = D_rB;
    else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_srcB = RESP;
    else d_srcB = RNONE;

    d_rvalA = get_Register(d_srcA);

    d_rvalB = get_Register(d_srcB);

    if (D_icode == IRRMOVL || D_icode == IIRMOVL || D_icode == IOPL) d_dstE = D_rB;
    else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_dstE = RESP;
    else d_dstE = RNONE;

    if (D_icode == IMRMOVL || D_icode == IPOPL) d_dstM = D_rA;
    else d_dstM = RNONE;

    SelFwdA_thread();

    FwdB_thread();

    d_icode = D_icode;
    d_ifun = D_ifun;
    d_stat = D_stat;
    d_valC = D_valC;
}

void CPU :: Execute_thread(){
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

    e_icode = E_icode;
    e_stat = E_stat;
    e_valA = E_valA;
    e_dstM = E_dstM;
}

void CPU :: Memory_thread(){
    if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL || M_icode == IMRMOVL) m_mem_addr = M_valE;
    else if (M_icode == IPOPL || M_icode == IRET) m_mem_addr = M_valA;
    else m_mem_addr = 0;

    if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL) m_mem_data = M_valA;
    else m_mem_data = 0;

    m_mem_read = (M_icode == IMRMOVL) || (M_icode == IPOPL) || (M_icode == IRET);
    m_mem_write = (M_icode == IRMMOVL) || (M_icode == IPUSHL) || (M_icode == ICALL);

    if (m_mem_read){
        mem_read(m_mem_addr, 4, m_valM, m_dimem_error);
    }

    if (m_mem_write){
        mem_write(m_mem_addr, 4, m_mem_data, m_dimem_error);
    }

    if (m_dimem_error) m_stat = SADR;
    else m_stat = M_stat;

    m_icode = M_icode;
    m_valE = M_valE;
    m_dstE = M_dstE;
    m_dstM = M_dstM;
}

void CPU :: Write_thread(){
    if (W_stat == SBUB) Stat = SAOK;
    else Stat = W_stat;

    if (Stat != SAOK) return;

    set_Register(W_dstE, W_valE);
    set_Register(W_dstM, W_valM);
}

void CPU :: Fetch_process(QProcess *F){
    std :: stringstream ss;
    ss << F_predPC << ' ' << M_icode << ' ' << M_Cnd << ' ' << M_valA << ' ' << W_icode << ' ' << W_valM << '\n';
    F_done = 0;
    QString OUTPUT = QString :: fromStdString(ss.str());
    F -> write(OUTPUT.toUtf8());
}

void CPU :: Decode_process(QProcess *D){
    std :: stringstream ss;
    ss << D_stat << ' ' << D_icode << ' ' << D_ifun << ' ' << D_rA << ' ' << D_rB << ' ' << D_valC << ' ' << D_valP << ' ' << M_dstM << ' '
       << M_dstE << ' ' << M_valE << ' ' << W_dstM << ' ' << W_valM << ' ' << W_dstE << ' ' << W_valE << '\n';
    D_done = 0;
    QString OUTPUT = QString :: fromStdString(ss.str());
    D -> write(OUTPUT.toUtf8());
}

void CPU :: Execute_process(QProcess *E){
    std :: stringstream ss;
    ss << E_stat << ' ' << E_icode << ' ' << E_ifun << ' ' << E_valC << ' ' << E_valA << ' ' << E_valB << ' ' << E_dstE << ' ' << E_dstM << ' '
       << E_srcA << ' ' << E_srcB << ' ' << (ZF?1:0) << ' ' << (SF?1:0) << ' ' << (OF?1:0) << ' ' << (CF?1:0) << '\n';
    E_done = 0;
    QString OUTPUT = QString :: fromStdString(ss.str());
    E -> write(OUTPUT.toUtf8());
}

void CPU :: Memory_process(QProcess *M){
    std :: stringstream ss;
    ss << M_stat << ' ' << M_icode << ' ' << M_Cnd << ' ' << M_valE << ' ' << M_valA << ' ' << M_dstE << ' ' << M_dstM << '\n';
    M_done = 0;
    QString OUTPUT = QString :: fromStdString(ss.str());
    M -> write(OUTPUT.toUtf8());
}

void CPU :: Write_process(QProcess *W){
    std :: stringstream ss;
    ss << W_stat << ' ' << W_icode << ' ' << W_valE << ' ' << W_valM << ' ' << W_dstE << ' ' << W_dstM << '\n';
    W_done = 0;
    QString OUTPUT = QString :: fromStdString(ss.str());
    W -> write(OUTPUT.toUtf8());
}

bool CPU :: F_ret(QProcess *F){
    std :: stringstream ss;
    char ch;
    ss.str("");
    ss << (F -> readAllStandardOutput()).toStdString();
    qDebug() << "F_ret";
//    qDebug() << QString :: fromStdString(ss.str());
    ss >> ch;
    if (ch == '*'){
        ss >> f_stat >> f_icode >> f_ifun >> f_rA >> f_rB >> f_valC >> f_valP >> f_PC >> f_predPC;
        F_done = 1;
    }
    else if (ch == '?'){
        int head, len, data;
        bool imem_error = 0;
        ss >> head >> len;
        mem_read(head, len, data, imem_error);
        ss.str("");
        ss << data << ' ' << (imem_error?1:0) << '\n';
        QString OUTPUT = QString :: fromStdString(ss.str());
//        qDebug() << OUTPUT;
        F -> write(OUTPUT.toUtf8());
    }
    else
        qDebug() << "F Error :" << ch;
    return F_done && D_done && E_done && M_done && W_done;
}

bool CPU :: D_ret(QProcess *D){
    std :: stringstream ss;
    char ch;
    ss.str("");
    ss << (D -> readAllStandardOutput()).toStdString();
    ss >> ch;
    if (ch == '*'){
        ss >> d_stat >> d_icode >> d_ifun >> d_valC >> d_valA >> d_valB >> d_dstE >> d_dstM >> d_srcA >> d_srcB >> d_rvalA >> d_rvalB >> D_marked_A_e >> D_marked_A_m >> D_marked_B_e >> D_marked_B_m;
        D_done = 1;
    }
    else if (ch == '?'){
        int src;
        ss >> src;
        int x = get_Register(src);
        ss.str("");
        ss << x << '\n';
        QString OUTPUT = QString :: fromStdString(ss.str());
        D -> write(OUTPUT.toUtf8());
    }
    else
        qDebug() << "D Error :" << ch;
    return F_done && D_done && E_done && M_done && W_done;
}

bool CPU :: E_ret(QProcess *E){
    std :: stringstream ss;
    char ch;
    ss.str("");
    ss << (E -> readAllStandardOutput()).toStdString();
    ss >> ch;
    if (ch == '*'){
        int z, s, o, c;
        ss >> e_stat >> e_icode >> e_Cnd >> e_valE >> e_valA >> e_dstE >> e_dstM >> e_aluA >> e_aluB >> e_alufun >> z >> s >> o >> c;
        ZF = z;
        SF = s;
        OF = o;
        CF = c;
        E_done = 1;
    }
    else
        qDebug() << "E Error :" << ch;
    return F_done && D_done && E_done && M_done && W_done;
}

bool CPU :: M_ret(QProcess *M){
    std :: stringstream ss;
    char ch;
    ss.str("");
    ss << (M -> readAllStandardOutput()).toStdString();
    ss >> ch;
    if (ch == '*'){
        ss >> m_stat >> m_icode >> m_valE >> m_valM >> m_dstE >> m_dstM;
        M_done = 1;
    }
    else if (ch == '?'){
        int head, len, data;
        bool imem_error = 0;
        ss >> head >> len;
        mem_read(head, len, data, imem_error);
        ss.str("");
        ss << data << ' ' << (imem_error?1:0) << '\n';
        QString OUTPUT = QString :: fromStdString(ss.str());
        M -> write(OUTPUT.toUtf8());
    }
    else if (ch == '!'){
        int head, len, data;
        bool imem_error = 0;
        ss >> head >> len >> data;
        mem_write(head, len, data, imem_error);
        ss.str("");
        ss << (imem_error?1:0) << '\n';
        QString OUTPUT = QString :: fromStdString(ss.str());
        M -> write(OUTPUT.toUtf8());
    }
    else
        qDebug() << "M_Error :" << ch;
    return F_done && D_done && E_done && M_done && W_done;
}

bool CPU :: W_ret(QProcess *W){
    std :: stringstream ss;
    char ch;
    ss.str("");
    ss << (W -> readAllStandardOutput()).toStdString();
    ss >> ch;
    if (ch == '*'){
        ss >> Stat;
        W_done = 1;
    }
    else if (ch == '!'){
        int src, val;
        ss >> src >> val;
        int x = set_Register(src, val);
        ss.str("");
        ss << x << '\n';
        QString OUTPUT = QString :: fromStdString(ss.str());
        W -> write(OUTPUT.toUtf8());
    }
    else
        qDebug() << "W_Error :" << ch;
    return F_done && D_done && E_done && M_done && W_done;
}

void CPU :: Forward_Deal(){
	e_set_cc = (E_icode == IOPL)
		&& !(W_stat == SADR || W_stat == SINS || W_stat == SHLT)
		&& !(M_stat == SADR || M_stat == SINS || M_stat == SHLT)
		&& !(m_stat == SADR || m_stat == SINS || m_stat == SHLT);
	
    if (!e_set_cc){
        ZF = _ZF;
        SF = _SF;
        OF = _OF;
        CF = _CF;
        cond(e_Cnd, ZF, SF, OF, CF);
    }
	
	if (D_marked_A_e && d_srcA == e_dstE)
		d_valA = e_valE;
	else if (D_marked_A_m && d_srcA == M_dstM)
		d_valA = m_valM;
	if (D_marked_B_e && d_srcB == e_dstE)
		d_valB = e_valE;
    else if (D_marked_B_m && d_srcB == M_dstM)
        d_valB = m_valM;
}
