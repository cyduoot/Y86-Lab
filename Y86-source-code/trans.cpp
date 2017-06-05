#include <bits/stdc++.h>

using namespace std;

char s[10010];
string sout;

int main(){
	freopen("ass_code.yo", "r", stdin);
	freopen("bin_code.txt", "w", stdout);
	sout = "";
	while (scanf("%s", s) != EOF){
		if (s[0] == '|'){
			char ch = getchar();
			while (ch != '\n' && ch != '\r')
				ch = getchar();
			continue;
		}
		if (s[0] == '0' && s[1] == 'x'){
			int ll = strlen(s) - 1;
			int x = 0;
			for (int i = 2; i < ll; i++)
				if (s[i] >= '0' && s[i] <= '9')
					x = (x << 4) + (s[i] - '0');
				else x = (x << 4) + (s[i] - 'a' + 10);
			x = x << 1;
			while (int (sout.length()) < x)
				sout = sout + '0';
			continue;
		}
		sout = sout + s;
	}
	int len = sout.length();
//	if (len & 1)
//		printf("\ntranslate Error\n");
//	cout << "start!" << endl;
//	cout << sout;
	for (int i = 0; i < len; i += 2){
		int t1 = (sout[i] >= '0' && sout[i] <= '9') ? sout[i] - '0' : sout[i] - 'a' + 10;
		int t2 = (sout[i + 1] >= '0' && sout[i + 1] <= '9') ? sout[i + 1] - '0' : sout[i + 1] - 'a' + 10;
		char ch = (t1 << 4) ^ t2;
		cout << ch;
	}
}
