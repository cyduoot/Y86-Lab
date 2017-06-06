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
