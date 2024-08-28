#include "rand.h"

static unsigned semilla = 1;

unsigned
generador(unsigned *num)
{
	unsigned int N = 0x7fffffff;
	unsigned int G = 48271u;

	unsigned int rem = *num % (N / G);
	unsigned int div = *num / (N / G);

	unsigned a = rem * G;
	unsigned b = div * (N % G);

	if (a > b) {
		*num = a - b;
	} else {
		*num = a + (N - b);
	}
	return *num;
}

// Generador de numeros pseudoaleatorios
// Basado en el generador de numeros pseudoaleatorios de Lehmer
// Un tipo de generador de numeros pseudoaleatorios de congruencial lineal
//
unsigned
get_random_num()
{
	return generador(&semilla);
}
