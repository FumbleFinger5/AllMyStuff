/*
class BitTable
{
public:
BitTable(DYNTBL *d);
Uint	get(int item);
void	upd(int item, Uint value);
~BitTable	();
private:
int		bits;
BitMap	*b;
};
*/

int bitwidth(ulong v);
int numbits(char *v, int ct, int len);

class BitStream
{
public:
BitStream::BitStream(char *addr);
BitStream::~BitStream();
int BitStream::get(int bitct);
void BitStream::put(ulong val, int bitct);
int	sz(void) {return((pos+7)/8);};
private:
char *a;
int pos;
};