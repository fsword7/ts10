// This program creates the register count table
// for VAX multi-stack instructions

#include <stdio.h>

int main(int argc, char **argv)
{
	int  idx1, idx2, t, cnt;
	char rcnt[32768];

	// Count registers each mask
	for (idx1 = 0; idx1 < 32768; idx1++) {
		cnt = 0;
		for (idx2 = 0, t = idx1; idx2 < 16; idx2++) {
			if (t & 1)
				cnt++;
			t >>= 1;
		}
		rcnt[idx1] = cnt << 2;
	}

	// Create 'C' array table
	printf("const uchar vax_rcntTable[] = {\n");
	for (idx1 = 0; idx1 < 32768;) {
		printf("\t");
		for (idx2 = 0; idx2 < 16; idx1++, idx2++)
			printf("%2d,", rcnt[idx1]);
		printf(" // %04X-%04X\n", idx1-16, idx1-1);
	}
	printf("};\n");
}
