#include "pdefs.h"
#include "BitTable.h"

/*
inline bool isPowerOf2(int i)
 {
   return i > 0 && (i & (i - 1)) == 0;
 } 
*/

int bitwidth(ulong v)	// Returns number of bits needed to store value 'v'
{
int i, bits;
for (bits=i=1;i<32;i++) if (v&(1<<i)) bits=i+1;
return(bits);
}

// 'v' is an array of 'ct' binary values (1, 2, or 4-bytes each, as passed in 'len')
// Find the highest value in the array, and return the number of bits needed to store it
int numbits(char *v, int ct, int len)
{
ulong val,hi=0;
while (ct--)
	if ((val=get_binary124(v,len,ct))>hi)
		hi=val;
return(bitwidth(hi));
}

/*
void BitTable::upd(int i, Uint v)
{
i*=bits;
for (int left=bits;left>0;left--, i++)
	{
	int on=b->is_on(i);
	if ((v>>left)&1)
		{if (!on) b->set(i);}
	else
		{if (on) b->unset(i);}
	}
}

Uint BitTable::get(int i)
{
Uint v=0;
i*=bits;
for (int left=bits;left>0;left--, i++)
	if (b->is_on(i))
		v|=(1<<left);
return(v);
}

BitTable::BitTable(DYNTBL *d)
{
char	*da=(char*)d->get(0);
bits=numbits(da,d->ct,d->sz);
b=new BitMap(bits*d->ct);
for (int i=0; i<d->ct;i++)
	upd(i,get_binary124(da,d->sz,i));
}

BitTable::~BitTable()
{
delete b;
};
*/

BitStream::BitStream(char *addr) {a=addr; pos=0;}

BitStream::~BitStream() {a=a;}

int BitStream::get(int bitct)
{
int ret=0;
if (!bitct)
	{
	while (pos&7) pos++;	// Align to the next 'byte' boundary on i/p
	ret=pos/8;				// This is the address of the string
	while (get(8)) {;}		// Step over the string (incl. EOS byte)
	}
else
	while (bitct--)
		if (bit_test(a,pos++)) ret|=(1<<bitct);
return(ret);
}

void BitStream::put(ulong val, int bitct)	// Write 'bitct' bits from 'val' to datastream
{
if (!bitct)					// If bitct==0, val is ptr->str - write the whole string (incl. EOS nullbyte)
	{
	while (pos&7) pos++;	// Align to the next 'byte' boundary on o/p
	char *p=(char*)val;
	do put(*p,8); while (*(p++));
	}
else
	while (bitct--)
		if (val&(1<<bitct)) bit_set(a,pos++);
		else bit_unset(a,pos++);
}