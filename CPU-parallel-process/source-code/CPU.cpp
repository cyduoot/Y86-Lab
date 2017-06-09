#include "CPU.h"
#include "simulator.h"
#include <sstream>

void CPU :: prepare(){
    if (F != NULL){
        F -> terminate();
        F -> waitForFinished();
        delete F;
    }
    if (D != NULL){
        D -> terminate();
        D -> waitForFinished();
        delete D;
    }
    if (E != NULL){
        E -> terminate();
        E -> waitForFinished();
        delete E;
    }
    if (M != NULL){
        M -> terminate();
        M -> waitForFinished();
        delete M;
    }
    if (W != NULL){
        W -> terminate();
        W -> waitForFinished();
        delete W;
    }

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

    F = new  QProcess();
    F->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/F.exe");
    connect(F, SIGNAL(readyRead()), this, SLOT(F_ret()));
    D = new  QProcess();
    D->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/D.exe");
    connect(D, SIGNAL(readyRead()), this, SLOT(D_ret()));
    E = new  QProcess();
    E->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/E.exe");
    connect(E, SIGNAL(readyRead()), this, SLOT(E_ret()));
    M = new  QProcess();
    M->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/M.exe");
    connect(M, SIGNAL(readyRead()), this, SLOT(M_ret()));
    W = new  QProcess();
    W->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/W.exe");
    connect(W, SIGNAL(readyRead()), this, SLOT(W_ret()));
}

std :: string int2str(int x){
    std :: string s;
    std :: stringstream ss;
    ss << x;
    s = ss.str();
    return s;
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

int CPU :: get_Register(int src){
    if (src < 8) return Reg[src];
    return 0xf;
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
//    E_op = E_op + "e_Cnd <- " + int2str(e_Cnd) + '\n';
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

void CPU :: Fetch(){
    std :: stringstream ss;
    ss << F_predPC << ' ' << M_icode << ' ' << M_Cnd << ' ' << M_valA << ' ' << W_icode << ' ' << W_valM;
    F_done = 0;
    std :: string OUTPUT = ss.str();
    qDebug() << OUTPUT.c_str();
    F -> write(OUTPUT.c_str());
}

void CPU :: Decode(){
    std :: stringstream ss;
    ss << D_stat << ' ' << D_icode << ' ' << D_ifun << ' ' << D_rA << ' ' << D_rB << ' ' << D_valC << ' ' << D_valP << ' ' << M_dstM << ' ';
    ss << M_dstE << ' ' << M_valE << ' ' << W_dstM << ' ' << W_valM << ' ' << W_dstE << ' ' << W_valE;
    D_done = 0;
    std :: string OUTPUT = ss.str();
    D -> write(OUTPUT.c_str());
}

void CPU :: Execute(){
    std :: stringstream ss;
    ss << E_stat << ' ' << E_icode << ' ' << E_ifun << ' ' << E_valC << ' ' << E_valA << ' ' << E_valB << ' ' << E_dstE << ' ' << E_dstM << ' ';
    ss << E_srcA << ' ' << E_srcB << ' ' << (ZF?1:0) << ' ' << (SF?1:0) << ' ' << (OF?1:0) << ' ' << (CF?1:0);
    E_done = 0;
    std :: string OUTPUT = ss.str();
    E -> write(OUTPUT.c_str());
}

void CPU :: Memory(){
    std :: stringstream ss;
    ss << M_stat << ' ' << M_icode << ' ' << M_Cnd << ' ' << M_valE << ' ' << M_valA << ' ' << M_dstE << ' ' << M_dstM;
    M_done = 0;
    std :: string OUTPUT = ss.str();
    M -> write(OUTPUT.c_str());
}

void CPU :: Write(){
    std :: stringstream ss;
    ss << W_stat << ' ' << W_icode << ' ' << W_valE << ' ' << W_valM << ' ' << W_dstE << ' ' << W_dstM;
    W_done = 0;
    std :: string OUTPUT = ss.str();
    W -> write(OUTPUT.c_str());
}

void CPU :: F_ret(){
    std :: stringstream ss;
    char ch;
    ss.clear();
    ss << (F -> readAllStandardOutput()).toStdString();
	ss >> ch;
    if (ch == '*'){
        ss >> f_stat >> f_icode >> f_ifun >> f_rA >> f_rB >> f_valC >> f_valP >> f_PC >> f_predPC;
        F_done = 1;
    }
    else if (ch == '?'){
        int head, len, data;
        bool imem_error;
        ss >> head >> len;
        mem_read(head, len, data, imem_error);
        ss.clear();
        ss << data << ' ' << (imem_error?1:0);
        F -> write(ss.str().c_str());
    }
    else{
        qDebug() << "F Error :" << ch;
    }
}

void CPU :: D_ret(){
    std :: stringstream ss;
    char ch;
    ss.clear();
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
        ss.clear();
        ss << x;
        D -> write(ss.str().c_str());
    }
	else
        qDebug() << "D Error :" << ch;
}

void CPU :: E_ret(){
    std :: stringstream ss;
    char ch;
    ss.clear();
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
}

void CPU :: M_ret(){
    std :: stringstream ss;
    char ch;
    ss.clear();
    ss << (M -> readAllStandardOutput()).toStdString();
    ss >> ch;
    if (ch == '*'){
        ss >> m_stat >> m_icode >> m_valE >> m_valM >> m_dstE >> m_dstM;
        M_done = 1;
    }
    else if (ch == '?'){
        int head, len, data;
        bool imem_error;
        ss >> head >> len;
        mem_read(head, len, data, imem_error);
        ss.clear();
        ss << data << ' ' << (imem_error?1:0);
        M -> write(ss.str().c_str());
    }
    else if (ch == '!'){
        int head, len, data;
        bool imem_error;
        ss >> head >> len >> data;
        mem_write(head, len, data, imem_error);
        ss.clear();
        ss << (imem_error?1:0);
        M -> write(ss.str().c_str());
    }
    else
        qDebug() << "M_Error :" << ch;
}

void CPU :: W_ret(){
    std :: stringstream ss;
    char ch;
    ss.clear();
    ss << (W -> readAllStandardOutput()).toStdString();
    ss >> ch;
    if (ch == '*'){
        ss >> Stat;
        W_done = 1;
    }
    else
        qDebug() << "W_Error :" << ch;
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
	else if (D_marked_B_m && d_srcA == M_dstM)
		d_valA = m_valM;
}

void CPU :: FFF(){
    _ZF = ZF;
    _SF = SF;
    _OF = OF;
    _CF = CF;
    Fetch();
	Decode();
	Execute();
	Memory();
	Write();
	Forward_Deal();
}
