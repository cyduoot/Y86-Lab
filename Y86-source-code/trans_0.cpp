#include <bits/stdc++.h>

using namespace std;

char s[110], rA[110], rB[110];
int x;
string sout;

bool same(string A, string B){
	return (A == B);
}

char get_num(char *s){
	if (same(s, "%eax")) return '0';
	if (same(s, "%ecx")) return '1';
	if (same(s, "%edx")) return '2';
	if (same(s, "%ebx")) return '3';
	if (same(s, "%esp")) return '4';
	if (same(s, "%ebp")) return '5';
	if (same(s, "%esi")) return '6';
	if (same(s, "%edi")) return '7';
//	printf("\ninvalid register : %s\n", s);
	return 'F';
}

string get_num(int v){
	string ss = "";
	char ch;
	int x;
	for (int i = 0; i < 4; i++){
		x = (v >> 4) & 0xf;
		if (x < 10) ch = '0' + x;
		else ch = 'a' + x - 10;
		ss = ss + ch;
		x = v & 0xf;
		if (x < 10) ch = '0' + x;
		else ch = 'a' + x - 10;
		ss = ss + ch;
		v >>= 8;
	}
	return ss;
}

void get_num(char *s, int &x){
	x = 0;
	if (s[0] >= '0' && s[0] <= '9'){
		int r = 0;
		while (s[r] >= '0' && s[r] <= '9'){
			x = x * 10 + s[r] - '0';
			r++;
		}
		r++;
		int l = 0;
		for (l = 0; s[l + r] != ')'; l++)
			s[l] = s[l + r];
		s[l] = 0;
	}
}

void trans(){
	if (same(s, "halt")){
		sout = sout + "00";
		return;
	}
	if (same(s, "nop")){
		sout = sout + "10";
		return;
	}
	if (same(s, "rrmovl")){
		scanf("%s%s", rA, rB);
		sout = sout + "20";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "irmovl")){
		scanf("%d%s", &x, rB);
		sout = sout + "30";
		sout = sout + 'F';
		sout = sout + get_num(rB);
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "rmmovl")){
		scanf("%s%s", rA, rB);
		get_num(rB, x);
		sout = sout + "40";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "mrmovl")){
		scanf("%s%s", rB, rA);
		get_num(rB, x);
		sout = sout + "50";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "addl")){
		scanf("%s%s", rA, rB);
		sout = sout + "60";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "subl")){
		scanf("%s%s", rA, rB);
		sout = sout + "61";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "andl")){
		scanf("%s%s", rA, rB);
		sout = sout + "62";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "xorl")){
		scanf("%s%s", rA, rB);
		sout = sout + "63";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "jmp")){
		scanf("%d", &x);
		sout = sout + "70";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "jle")){
		scanf("%d", &x);
		sout = sout + "71";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "jl")){
		scanf("%d", &x);
		sout = sout + "72";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "je")){
		scanf("%d", &x);
		sout = sout + "73";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "jne")){
		scanf("%d", &x);
		sout = sout + "74";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "jge")){
		scanf("%d", &x);
		sout = sout + "75";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "jg")){
		scanf("%d", &x);
		sout = sout + "76";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "cmovle")){
		scanf("%s%s", rA, rB);
		sout = sout + "21";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "cmovl")){
		scanf("%s%s", rA, rB);
		sout = sout + "22";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "cmove")){
		scanf("%s%s", rA, rB);
		sout = sout + "23";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "cmovne")){
		scanf("%s%s", rA, rB);
		sout = sout + "24";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "cmovge")){
		scanf("%s%s", rA, rB);
		sout = sout + "25";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "cmovg")){
		scanf("%s%s", rA, rB);
		sout = sout + "26";
		sout = sout + get_num(rA);
		sout = sout + get_num(rB);
		return;
	}
	if (same(s, "call")){
		scanf("%d", &x);
		sout = sout + "80";
		sout = sout + get_num(x);
		return;
	}
	if (same(s, "ret")){
		sout = sout + "90";
		return;
	}
	if (same(s, "pushl")){
		scanf("%s", rA);
		sout = sout + "A0";
		sout = sout + get_num(rA);
		sout = sout + 'F';
		return;
	}
	if (same(s, "popl")){
		scanf("%s", rA);
		sout = sout + "B0";
		sout = sout + get_num(rA);
		sout = sout + 'F';
		return;
	}
//	printf("\ninvalid instruction : %s\n", s);
}

int main(){
	freopen("ass_code.txt", "r", stdin);
	freopen("bin_code.txt", "w", stdout);
	while (scanf("%s", s) != EOF){
		trans();
//		cout << sout << endl;
	}
//	cout << sout;
	int len = sout.length();
/*	if (len & 1)
		printf("\ntranslate Error\n");
//	cout << "start!" << endl;*/
	for (int i = 0; i < len; i += 2){
		int t1 = (sout[i] >= '0' && sout[i] <= '9') ? sout[i] - '0' : sout[i] - 'a' + 10;
		int t2 = (sout[i + 1] >= '0' && sout[i + 1] <= '9') ? sout[i + 1] - '0' : sout[i + 1] - 'a' + 10;
		char ch = (t1 << 4) ^ t2;
		cout << ch;
	}
}
