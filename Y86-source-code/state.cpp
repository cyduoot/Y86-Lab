#include <bits/stdc++.h>
#include "CPU.h"
using namespace std;

CPU cpu;

void pri_Reg_state(){
	printf("%%eax : %x\n", cpu.Reg[0]);
	printf("%%ecx : %x\n", cpu.Reg[1]);
	printf("%%edx : %x\n", cpu.Reg[2]);
	printf("%%ebx : %x\n", cpu.Reg[3]);
	printf("%%esp : %x\n", cpu.Reg[4]);
	printf("%%ebp : %x\n", cpu.Reg[5]);
	printf("%%esi : %x\n", cpu.Reg[6]);
	printf("%%edi : %x\n", cpu.Reg[7]);
	printf("Stat : %d\n", cpu.Stat);
	printf("----------------------------\n");
}

void pri_mem_state(){
	map <int , int> :: iterator cp;
	for (cp = mem.begin(); cp != mem.end(); cp++)
		printf("%08x : %02x\n", cp -> first, cp -> second);
}

int main(){
//	freopen("cpu.log", "w", stdout);
	cpu.read_in();
	cpu.prepare();
	while (cpu.Stat == SAOK){
//		pri_Reg_state();
		cpu.circle_time++;
		cpu.Control();
		cpu.Send();
		cpu.Write();
		cpu.Memory();
		cpu.Execute();
		cpu.Decode();
		cpu.Fetch();
	}
	pri_Reg_state();
//	pri_mem_state();
}
