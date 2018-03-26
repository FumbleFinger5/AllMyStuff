#include "pdefs.h"
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include "dll.h"

int first_leak;

static int		uniq;

static TAG		*logger;
static DYNTBL	*log;


#define MAXCT 65000		// 02/12/02

#ifdef MEMLOG		// see pdecs.h
#define FST	12		// pre-25/09/02 was 6 (size was 2-byte short, not 4-byte int)
#define LST 4		// 4
#else
#define FST	0
#define LST 0
#endif
#define EXTRA_BYTES (FST+LST)

static void **a;
static int ct,c1;

/*		ex malloc.h
#define _HEAPEMPTY      (-1)
#define _HEAPOK         (-2)
#define _HEAPBADBEGIN   (-3)
#define _HEAPBADNODE    (-4)
#define _HEAPEND        (-5)
#define _HEAPBADPTR     (-6)
*/
int do_memchk;
void memchk(char *txt)
{
if (!do_memchk) return;
#ifdef MEMLOG
int totsiz=0;
for (int i=0;i<ct;i++)
	{
	char *c=(char*)a[i];
	long siz=*(long*)c;
	totsiz+=siz;
	if ((*(long*)&c[FST-4]!=0x12345678 || *(long*)&c[siz+FST]!=0x87654321))
		{
		char w[80];
		SetErrorText(strfmt(w,"Corrupt memory:%s",txt));
		throw SE_MEMBAD;
		}
	}
int open_file_handles_ct(void);
char w[128];
strfmt(w,"Open Files:%ld, Objects:%ld, MemUsed:%ld",open_file_handles_ct(),log->ct,totsiz);
sjhloG("%s %s",txt,w);
#endif
int rc=_heapchk();
if (rc==87687) sjhLog("Memlog Error %s",txt);			// what is -2  ???
return;
}
/*&#define _HEAPEMPTY      (-1)
#define _HEAPOK         (-2)
#define _HEAPBADBEGIN   (-3)
#define _HEAPBADNODE    (-4)
#define _HEAPEND        (-5)
#define _HEAPBADPTR     (-6)
#define _FREEENTRY      0
#define _USEDENTRY      1*/


#ifdef MEMLOG
#define LUMP 1024		// Chunk size for extending 'a'
static void*  log_give(void *p, Uint siz)	// *p==REAL pointer	// Log this block as 'allocated'
{																// and return address of "app-level data portion" of block
char *c=(char*)p;
Uint *u=(Uint*)p;
u[0]=siz;
u[1]=++uniq; 
if (uniq==270)
uniq=uniq;	// LINK01 **************** run to cursor HERE to Find when unique sequence number of this mem_leak was allocated
*(long*)&c[FST-4]=0x12345678;				// Put our special values before and after the address returned to app,
*(long*)&c[siz+FST]=0x87654321;				// so we can check for buffer over/underrun when it's released

if (ct>=MAXCT) throw SE_OUTOFHEAP;
if (c1<=ct)
	{
	c1+=LUMP;
	a=(void**)(ct?realloc(a,c1*sizeof(void*)):calloc(c1,sizeof(void*)));
	}
a[ct++]=p;
return(&c[FST]);
}

static void  log_take(const void *p)	// *p==PSEUDO pointer from App	// Remove this block from the
{																		// table of 'allocated' addresses
char *c=((char*)p)-FST;					// (get the REAL pointer to allocated memory block)
long siz=*(Uint*)c;
if ((*(long*)&c[FST-4]!=0x12345678 || *(long*)&c[siz+FST]!=0x87654321))
	{
	SetErrorText("Corrupt memory block over/underrun");
	throw SE_MEMBAD;
	}
for (int	i=ct;i--;)
	if (a[i]==c)
		{
		if (--ct>i) memmove(&a[i],&a[i+1],(ct-i)*sizeof(void*));
		return;
		}
throw SE_MEMPTR;
}
#else
#define log_give(p,siz) (p)
#endif


static void* give(Uint siz)
{
if (!siz)
	throw SE_BADALLOC;
void *p=calloc(siz+EXTRA_BYTES,1);
return(log_give(p,siz));
}

void *memrealloc(void *data, Uint siz)		// *data==PSEUDO pointer
{
#ifdef MEMLOG
if (data) log_take(data);
#endif
if (!siz) throw SE_BADALLOC;
char *c=(char*)data; if (c) c-=FST;
c=(char*)realloc(c,siz+EXTRA_BYTES);
return(log_give(c,siz));
}
void memtake(const void *data)
{
if(data)
	{
#ifdef MEMLOG
	log_take(data);
#endif
	free((void*)(((char*)data)-FST));
	}
}


void *memgive(Uint siz) 
{
return(give(siz));
}

void *memadup(const void *data, Uint siz)
{
return(memmove(memgive(siz),data,siz));
}


void	scrap(void **pointer)
{
if (*pointer)
	{
	memtake(*pointer);
	*pointer=0;
	}
}

int memtakeall(void)						// Release all blocks allocated by
{											// by memgive() or memrealloc()
int leak=ct;
while (ct)
	{
#ifdef MEMLOG
		{
		int *pi=(int*)a[ct-1];
//		int sz=pi[0];				// get the (app-requested) allocated block size,
		int sq=pi[1];				// - unique sequence number, (of this mem_leak)
		if (!first_leak)
			first_leak=sq;			// LINK01 
//		char *data=(char*)&pi[3];	// - and 'pointer to app-perceived data area'
		}							// so we can track down where it got allocated by the app
#endif
	memtake(((char*)(a[ct-1]))+FST);		// (memtake decrements ct)
	}
if (c1)
	{
	c1=0;
	free(a);
	}
return(leak);
}


#define T(i) &((char*)tbl)[(i)*sz]
int in_table(int *p, const void *ky, void *tbl, int c, int sz, PFI_v_v cmp)
{
int	m,lo,hi,i;
if (!p) p=&i;					// 'dummy' address so it's not NULLPTR
m=lo=0;
hi=c-1;
while (lo<=hi)
	{
	m=((long(hi)+lo)/2);
	if ((i=(cmp)(ky,T(m)))==0) return(*(p)=m);
	else if (i<0) hi=m-1; else lo=m+1;
	}
*(p)=m+(m<c && (cmp)(ky,T(m))>0);
return(NOTFND);
}


// Put 'ky' into table 'tbl', keeping the table sorted as per 'cmp()',
// if it isn't already present. Return the subscript of the existing
// position of 'ky' if it was already in table, or at which it was added
// if it wasn't (in which case '*ct' is incremented. Note that comparator
// doesn't necessarily compare 'sz' bytes (it may be less), but if a key
// is added, it WILL add that many bytes.

// WARNING!		Don't use the slot tbl[ct] as workspace for 'ky', because
//					it gets overwritten BEFORE being moved into the table!

int to_table(void *ky, void *tbl, int *ct, int sz, PFI_v_v cmp)
{
int	m,c;
if (in_table(&m,ky,tbl,c=*ct,sz,cmp)==NOTFND)
	{if (m<c) memmove(T(m+1),T(m),(c-m)*sz); 	(*ct)++; memmove(T(m),ky,sz);}
return(m);
}
#undef T

int to_table_s(void *ky, void *tbl, short *ct, int sz, PFI_v_v cmp)
{
int	c=*ct, ret=to_table(ky,tbl,&c,sz,cmp);
*ct=(short)c;
return(ret);
}

void DYNAG::init(int _sz, int _ct)
{
len=_sz;
eot=ct=0;
_cargo=0;
slave=NULL;
if (len)
	{
	if (!_ct) _ct=16; // start with space for 16 items unless we know how many we'll want
	sz=len * _ct;
	}
else		// create Slave dynag of offsets to strings if vari-length items
	{
	slave=new DYNAG(sizeof(int),_ct);
	sz=128;
	}
a=(char *)memgive(sz);
}

DYNAG::DYNAG(int _sz, int _ct)
{
init(_sz,_ct);
}

DYNAG::DYNAG(DYNAG *org)	// 'Copy' constructor initialised from existing DYNAG
{
init(org->len,org->ct);
for (int i=0;i<org->ct;i++) put(org->get(i),i);
if (org->_cargo) cargo(org->cargo(0),org->cargo_sz);
}


DYNAG::~DYNAG(void)
{
Scrap(_cargo);
memtake(a);
SCRAP(slave);
}


void* DYNAG::put(const void *item, int n)	// Put an item into array. If there's not enough room, increase by another chunk
{
int this_len;			// Avoids 'bitty' allocs, but minimises wasted 'slack'
char *ret;				// Use put(0) to eliminate slack completely at any time
if (!item)
	{
	a=(char *)memrealloc(a,(sz=eot)+!eot);
	return(0);
	}	// avoid 'eot==0'!
if (len) this_len=len; else this_len=strlen((char*)item)+1;
if (eot+this_len>sz)
	{
	int newsz=sz+(sz/4+this_len+(len?(len*8):64));
	a=(char *)memrealloc(a,sz=newsz);
	}
ret=&a[eot];	// save current end-of-table for return value
eot+=this_len;			// Increase current 'used' table length
if (n>=ct) n=NOTFND;
if (n>=0)
	{
	ret=(char *)get(n);
	memmove(&ret[this_len],ret,eot-(ret-a)-this_len);
	}
else n=ct;
if (!len)								// If adding entry to variable-length (len=0) DYNAG,
	{									// bump up offsets in any following entries in Slave
	int offset=ret-a;
	int *q=(int*)slave->put(&offset,n);
	while (n++<ct)
		(*(++q))+=this_len;
	}
ct++;
return(memmove(ret,item,this_len));	// (put new item in and return its address)
}

void* DYNAG::puti(int i) {return(put(&i));}		// (just a little wrapper because we often store 1-2-4 byte int's in tables)

void *DYNAG::get(int n)
{
void *addr=0;
if (n>=0 && n<ct)
	{
	if (len)
		addr=&a[n*len];
	else
		addr=&a[*(int*)slave->get(n)];
	}
return(addr);
}

void DYNAG::del(int n)
{
char *p=(char *)get(n);
if (p)
	{
	int to=long(p)-long(a),this_len=len;
	ct--;
	if (!this_len)
		{
		this_len=strlen(p)+1;
		slave->del(n);
		for (int x=n;x<ct;x++) *(int*)slave->get(x)-=this_len;
		}
	if ((n=eot-(to+this_len))>0) memmove(&a[to],&a[to+this_len],n);
	eot-=this_len;
	}
}

int DYNAG::in(const void *item)	// Return subscript of item in array, or NOTFND
{
char *p;
for (int n=0;(p=(char*)get(n))!=NULL;n++)
	{
	switch (len)
		{
		case 0: if (!strcmp(p, (char*)item)) return(n); break;
		case 1: if (p[0]==*(char*)item) return(n); break;
		case 2: if (SAME2BYTES(p,item)) return(n); break;
		case 4: if (SAME4BYTES(p,item)) return(n); break;
		default: if (!memcmp(p,item,len)) return(n); break;
		}
	}
return(NOTFND);
}

void* DYNAG::cargo(void *data, int sz)
{
if (sz)
	{
	Scrap(_cargo);
	if (data) _cargo=memadup(data,cargo_sz=sz);
	}
return(_cargo);
}



DYNTBL::DYNTBL(int _sz, PFI_v_v _cp, int _ct):DYNAG(_sz,_ct)
{
cp=_cp;
}

int DYNTBL::set_cp(PFI_v_v _cp)
{
qsort(a,ct,len,cp=_cp);
for (int i=0; i<ct-1; i++)
	if ( (cp)(&a[i*len],&a[(i+1)*len])>=0)
		return(NO);	// ??? - there must be a duplicate key!
return(YES);
}

void* DYNTBL::find(const void *k)
{
return(get(in(k)));
}


int DYNTBL::in(const void *k)
{
if (!len) return(DYNAG::in(k));
return(in_table(0,k,a,ct,len,cp));
}

int DYNAG::in_or_add(const void *k)
{
int i=in(k);
if (i==NOTFND) {i=ct; put(k);}
return(i);
}

int DYNTBL::in_or_add(const void *k)
{
int i=in(k);
if (i==NOTFND)
	{
	put(k);
	i=in(k);
	}
return(i);
}

int DYNTBL::in_GE(const void *k)
{
if (!len) m_finish("SysErr 963");
int p, exact=in_table(&p,k,a,ct,len,cp);	// If exact key doesn't exist, 'p' = position it would be at (i.e. BK_GE)
if (exact==NOTFND && p==ct) p=NOTFND;
return(p);
}

static const char *_aaa, *_kkk;
static int _cdecl cp_slave(int *a, int *b)
{
return(strcmp((*a==NOTFND)?_kkk:&_aaa[*a],(*b==NOTFND)?_kkk:&_aaa[*b]));
}

int DYNAG::find_str(int *p, const void *k)
{
_aaa=a;
_kkk=(const char*)k;
int fix=-1;
int exact=in_table(p,&fix,slave->a,ct,4,(PFI_v_v)cp_slave);	// If exact key doesn't exist, 'p' = position it would be at (i.e. BK_GE)
if (exact==NOTFND && *p==ct) *p=NOTFND;
return(exact);
}

void* DYNTBL::put(const void *k)
{
int p=NOTFND;
if (!len)			// TO DO: find where this is used (to maintain sorted table of text strings)
	{
	int exact=find_str(&p,k);	// If exact key doesn't exist, 'p' = position it would be at (i.e. BK_GE)
	if (exact!=NOTFND) return(get(exact));
	return(DYNAG::put(k,p));
	}
if (!cp || in_table(&p,k,a,ct,len,cp)==NOTFND)	// 050108 - just 'append' if comparator not yet set
	return(DYNAG::put(k,p));
return(&a[len*p]);
}

void* DYNTBL::puti(int i) {return(put(&i));}		// (just a little wrapper because we often store 1-2-4 byte int's in tables)

void DYNTBL::del(int n)
{
DYNAG::del(n);
}

void DYNTBL::merge(DYNTBL *add)
{
if (len!=add->len) throw(SE_MEMTBL);
for (int i=0;i<add->ct;i++) put(add->get(i));
}

short *geti2(short *i2, DYNAG *d)	// Copy FIRST & LAST (short) values from 'd' to 2-element array i2
{
i2[0]=*(short*)(d->get(0));
i2[1]=*(short*)(d->get(d->ct-1));
return(i2);
}


// On first call, if 'cargo' is not set within the DYNAG we'll make it a ptr -> int initially containing 0
// If calling code already created 'cargo', it MUST ensure it points to a zeroised int for this fn.
int return_up_to_64K(char *buffer, DYNAG *tbl, void (*fun)(const void*,char*))
{
char	wrk[2048];
void	*p;
int		n,*pn=(int*)tbl->cargo(0,n=0);
if (!pn) pn=(int*)tbl->cargo(&n,sizeof(int));
*buffer=0;
while ((p=tbl->get(*pn))!=0)
	{
	(fun)(p, wrk);
	if (strlen(buffer)+strlen(wrk)>=VBUFSZ)
		break;
	(*pn)++;			/// THIS is where *pn WAS incremented!!!!!
	strcat(buffer,wrk);
	}
return(*buffer);
}

static int zct;

TAG::TAG()
{
if (log)
	{
	id=++zct;
if (id==32)
{
//assert(0);
id=id;						// run to cursor HERE to Find when class_leak 'id' was instantiated
}
	void *ptr=(void*)this;
	log->put(&ptr);
	}
else id=NO;
}

TAG::TAG(int first)
{
first=first; // ??????? what's this param for ?????
log=new DYNTBL(4,cp_long);
id=NOTFND;
}

int TAG::leaks(void)
{
DYNTBL *w=log;
log=0;
int i,ct;
for (i=ct=0;i<w->ct;i++)
	{
	void *v=w->get(i);
	TAG *t=*(TAG**)v;
	int leak=t->id;
if (!first_leak)
{
first_leak=leak*10+1;
//assert(first_leak!=11);
}
	ct++;
	}
delete w;
return(ct);
}

TAG::~TAG()
{
if (log)
	{
	if (id>0)
		{
		void *ptr=(void*)this;
		int ck=log->ct;
		log->del(log->in(&ptr));
		if (log->ct!=ck-1) m_finish("dammit");
		}
	if (id==NOTFND)
		{
		if (leaks())
			m_finish("Logged Class instances not released!");
		}
	}
}

int leak_tracker(int start)
{
#ifndef MEMLOG
return(0);
#endif
if (start)
	{
	assert(logger==0);
	logger=new TAG(YES);
	return(0);
	}

assert(logger!=0);
void nolog_rls(void);
nolog_rls();
int class_leak=logger->leaks();
SCRAP(logger);
int fleaks=flcloseall();				// any non=closed files?
int mem_leak=memtakeall();
return(class_leak+fleaks+mem_leak);

}


void swap_data(void *a, void *b, int sz)
{
char wrk[8],*w;
if (sz<=8) w=wrk; else w=(char*)memgive(sz);
memmove(w,a,sz);
memmove(a,b,sz);
memmove(b,w,sz);
if (sz>8) memtake(w);
}

