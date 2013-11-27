#ifndef LIBVEB_H
#define LIBVEB_H

typedef unsigned int uint;
typedef unsigned char uchar;
typedef struct Veb Veb;

struct Veb
{
	uchar *D;
	uint k;
	uint M;
};

Veb vebnew(uint, int);
uint vebsize(uint);
void vebdel(Veb, uint);
void vebput(Veb, uint);
uint vebsucc(Veb, uint);
uint vebpred(Veb, uint);

#endif
