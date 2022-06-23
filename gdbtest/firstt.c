#include <stdio.h>

int global_var;

void change_var() {
	global_var = 100;
}

int main() {
	change_var();
	return 0;
}
