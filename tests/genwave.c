// genwave.c 

#include <stdio.h>
#include <math.h>

#define N 1746 

int main(void) {
	int x;
	printf("const short int wavetable[%d] = {\n", N);
	for(x=0; x<N; x++) {
		// 2^16 / 2 = 32768
		unsigned short int value = 32760 + 32760 * sin(2 * M_PI * x / N); 
			//sin() returns a double
		printf("%d, ", value);
		if ((x % 8) == 7) 
			printf("\n");        
	}
	printf("};\n");
}
