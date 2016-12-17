#include <bits/stdc++.h>
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

#define SBUB 0 //bubble

const int code_head = 0x00000000;
const int MAXLEN = 10000000;

map <int, int> mem;

class CPU{
	public:
	
	char bin_code[MAXLEN];
	int circle_time;
	
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
	
	void prepare(){
		memset(this, 0, sizeof(*this));
		f_predPC = code_head;
		f_icode = d_icode = e_icode = m_icode = 1;
		f_stat = d_stat = e_stat = m_stat = Stat = 1;
		f_rA = f_rB = RNONE;
		d_dstE = d_dstM = d_srcA = d_srcB = RNONE;
		e_dstE = e_dstM = RNONE;
		m_dstE = m_dstM = RNONE;
	}
	
	void mem_read(int head, int len, int &data, bool &imem_error){
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
	
	void mem_read_try(int head, int len, int &data, bool &imem_error){
		for (int i = 0; i < len; i++)
			if (head + i < 0){
				imem_error = 1;
				break;
			}
	}
	
	void mem_write(int head, int len, int data, bool &imem_error){
		for (int i = 0; i < len; i++)
			if (head + i >= 0){
				mem[head + i] = data & (0xff);
				data >>= 8;
			}else{
				imem_error = 1;
				break;
			}
	}
	
	void mem_write_try(int head, int len, int data, bool &imem_error){
		for (int i = 0; i < len; i++)
			if (head + i < 0){
				imem_error = 1;
				break;
			}
	}
	
	void read_in(){
//		freopen("bin_code.txt", "r", stdin);
		FILE *bin_stream;
		int head = code_head;
		bin_stream = fopen("bin_code.txt", "r");
		int len = fread(bin_code, sizeof(char), MAXLEN, bin_stream);
		for (int i = 0; i < len; i++){
			mem_write(head, 1, bin_code[i], imem_error);
			head++;
		}
		//补 halt
//		mem_write(head, 1, 0x00, imem_error);
//		head++;
	}

	void select_PC(){
		if (M_icode == IJXX && !M_Cnd) f_PC = M_valA;
		else if (W_icode == IRET) f_PC = W_valM;
		else f_PC = F_predPC;
	}
	
	void Split(int f_PC, int &f_a, int &f_b, bool &imem_error){
		int x;
		f_a = f_b = 0;
		mem_read(f_PC, 1, x, imem_error);
		if (imem_error) return;
		
		f_a = x >> 4;
		f_b = x & 0xf;
	}
	
	void SelFwdA(){
		if (D_icode == ICALL || D_icode == IJXX) d_valA = D_valP;
		else if (d_srcA == e_dstE) d_valA = e_valE;
		else if (d_srcA == M_dstM) d_valA = m_valM;
		else if (d_srcA == M_dstE) d_valA = M_valE;
		else if (d_srcA == W_dstM) d_valA = W_valM;
		else if (d_srcA == W_dstE) d_valA = W_valE;
		else d_valA = d_rvalA;
	}
	
	void FwdB(){
		if (d_srcB == e_dstE) d_valB = e_valE;
		else if (d_srcB == M_dstM) d_valB = m_valM;
		else if (d_srcB == M_dstE) d_valB = M_valE;
		else if (d_srcB == W_dstM) d_valB = W_valM;
		else if (d_srcB == W_dstE) d_valB = W_valE;
		else d_valB = d_rvalB;
	}
	
	int get_Register(int src){
		if (src < 8) return Reg[src];
		return 0xf;//why ??
	}
	
	void set_Register(int src, int val){
		if (src < 8) Reg[src] = val;
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
	}
	
	void ALU(int alua, int alub, int alufun, int &valE, bool setCC, bool &ZF, bool &SF, bool &OF, bool &CF){
		if (alufun == 0) valE = alua + alub;
		else if (alufun == 1) valE = alub - alua;//changed
		else if (alufun == 2) valE = alua & alub;
		else if (alufun == 3) valE = alua ^ alub;
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
	}
	
	void D_prepare(int &d_srcA, int &d_srcB){
		if (D_icode == IRRMOVL || D_icode == IRMMOVL || D_icode == IOPL || D_icode == IPUSHL) d_srcA = D_rA;
		else if (D_icode == IPOPL || D_icode == IRET) d_srcA = RESP;
		else d_srcA = RNONE;
		
		if (D_icode == IOPL || D_icode == IRMMOVL || D_icode == IMRMOVL) d_srcB = D_rB;
		else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_srcB = RESP;
		else d_srcB = RNONE;
	}
	
	void E_prepare(bool &need_use_ALU, int &e_aluA, int &e_aluB, int &e_alufun, bool &e_set_cc, int &e_Cnd, int &e_valE, 
					bool &ZF, bool &SF, bool &OF, bool &CF){
		
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
	}
	
	void M_prepare(int &m_mem_addr, int &m_mem_data, bool &m_mem_read, bool &m_mem_write){
		if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL || M_icode == IMRMOVL) m_mem_addr = M_valE;
		else if (M_icode == IPOPL || M_icode == IRET) m_mem_addr = M_valA;
		else m_mem_addr = 0;
		
		if (M_icode == IRMMOVL || M_icode == IPUSHL || M_icode == ICALL) m_mem_data = M_valA;
		else m_mem_data = 0;
		
		m_mem_read = (M_icode == IMRMOVL) || (M_icode == IPOPL) || (M_icode == IRET);
		m_mem_write = (M_icode == IRMMOVL) || (M_icode == IPUSHL) || (M_icode == ICALL);
	}
	
	void F_Control(){
		F_stall = (E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB);
		F_stall |= (D_icode == IRET || E_icode == IRET || M_icode == IRET);
	}
	
	void D_Control(){
		D_stall = (E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB);
		D_bubble = (E_icode == IJXX && !e_Cnd);
		D_bubble |= (!((E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB))) && (IRET == D_icode || IRET == E_icode || IRET == M_icode);
	}
	
	void E_Control(){
		E_bubble = (E_icode == IJXX && !e_Cnd);
		E_bubble |= (E_icode == IMRMOVL || E_icode == IPOPL) && (E_dstM == d_srcA || E_dstM == d_srcB);
	}
	
	void M_Control(){
		M_bubble = (m_stat == SADR || m_stat == SINS || m_stat == SHLT);
		M_bubble |= (W_stat == SADR || W_stat == SINS || W_stat == SHLT);
	}
	
	void W_Control(){
		W_stall = (W_stat == SADR || W_stat == SINS || W_stat == SHLT);
	}
	
	void Control(){
		F_Control();
		D_Control();
		E_Control();
		M_Control();
		W_Control();
	}
	
	void Send(){
		
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
	
	void Fetch(){
//		if (F_stall) return;
		
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
	
	void Decode(){
//		if (D_stall) return;
		
		D_prepare(d_srcA, d_srcB);
		
		d_rvalA = get_Register(d_srcA);
		
		d_rvalB = get_Register(d_srcB);
		
		if (D_icode == IRRMOVL || D_icode == IIRMOVL || D_icode == IOPL) d_dstE = D_rB;
		else if (D_icode == IPUSHL || D_icode == IPOPL || D_icode == ICALL || D_icode == IRET) d_dstE = RESP;
		else d_dstE = RNONE;
		
		if (D_icode == IMRMOVL || D_icode == IPOPL) d_dstM = D_rA;
		else d_dstM = RNONE;
		
		SelFwdA();
		
		FwdB();
		
		d_icode = D_icode;
		d_ifun = D_ifun;
		d_stat = D_stat;
		d_valC = D_valC;
	}
	
	void Execute(){
//		if (E_stall) return;
		
		E_prepare(need_use_ALU, e_aluA, e_aluB, e_alufun, e_set_cc, e_Cnd, e_valE, ZF, SF, OF, CF);
		
		if (E_icode == IRRMOVL && !e_Cnd) e_dstE = RNONE;
		else e_dstE = E_dstE;
		
		e_icode = E_icode;
		e_stat = E_stat;
		e_valA = E_valA;
		e_dstM = E_dstM;
	}
	
	void Memory(){
//		if (M_stall) return;
		
		M_prepare(m_mem_addr, m_mem_data, m_mem_read, m_mem_write);
		
		if (m_mem_read)
			mem_read(m_mem_addr, 4, m_valM, m_dimem_error);
		
		if (m_mem_write)
			mem_write(m_mem_addr, 4, m_mem_data, m_dimem_error);
		
		if (m_dimem_error) m_stat = SADR;
		else m_stat = M_stat;
		
		m_icode = M_icode;
		m_valE = M_valE;
		m_dstE = M_dstE;
		m_dstM = M_dstM;
	}
	
	void Write(){
		if (W_stat == SBUB) Stat = SAOK;
		else Stat = W_stat;
		
		if (Stat != SAOK) return;
		
		set_Register(W_dstE, W_valE);
		set_Register(W_dstM, W_valM);
	}
};

