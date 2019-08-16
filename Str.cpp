#include "pdefs.h"
#include "dll.h"
#include <stdarg.h>
#include <stdio.h>

int a2err;			// 0 = a2l() succeeded, 1 = a2l() failed (hit non-digit as stored in a2err_char), 2 = failed with empty i/p string
int	a2err_char;

long rnd_seed;
short rnd(short lo,short hi)
{
if(hi<=lo)return lo;
rnd_seed = 1664525L*rnd_seed + 907612489L;
return((short)((double)((rnd_seed>>1)&0x7fffffff)/2147483648.0*(hi-lo+1)+lo));
}


void  __stdcall XStatusCallback(int done, int total)
{
static HDL tmr=0;
if (done==-1 && total==-1) {if (tmr) tmrrls(tmr); tmr=0; return;}
if (!tmr) tmr=tmrist(0);
if (tmrelapsed(tmr)>150 || done==total)
	{
	if (done>=0)
		{
		printf("done %d of %d\n",done,total);
		}
	tmrrls(tmr); tmr=0;
	}
}

void	__stdcall Xgetcwd(char *cwd);

static void echo1(const char *fmt,...)
{
static	HDL f=0;
char	wb[1024],*w=wb;
if (!fmt)
	{
	if (f)
		{
		char cwd[255];
		Xgetcwd(cwd);
		printf("\nThis text echoed to %s\\%s\n",cwd,flnam(f));
		flclose(f);
		}
	f=0;
	return;
	}
if (!f) f=flopen_trap("WXP.LOG","w");
if (stridxc('%',fmt)==NOTFND) w=(char *)fmt;
else {va_list va; va_start(va,fmt);_strnfmt(w,sizeof(wb)-1,fmt,va);}
int i,ch,cr;
for (i=cr=0;(ch=w[i])!=0;w++)
	{
	if (ch==LNFEED) if (!cr) flputc('\r',f);
	cr=(ch==CRET);
	flputc(ch,f);
	}
flputs(w,f);
}

void __stdcall Xecho(const char *fmt,...)
{
char	wb[1024],*w=wb;
if (!fmt) {echo1(0); return;}
if (stridxc('%',fmt)==NOTFND) w=(char *)fmt;
else {va_list va; va_start(va,fmt);_strnfmt(w,sizeof(wb)-1,fmt,va);}
printf("%s",w);
echo1("%s",w);
}


int short_overlap(short *a, short *b)
{
if (*((int*)a) && *((int*)b))					// If EITHER range is 0-0 it MUST be an overlap!
	if (a[0]>b[1] || a[1]<b[0]) return(NO);		// (else it's a pair of scalar ranges to be checked)
return(YES);
}


bool in_rng(short val, short*rng)
{
return(val>=rng[0] && val<=rng[1]);
}

int force_into_range(int value, int min, int max)
{
if (max<min) max=min;
if (value<min) value=min;
if (value>max) value=max;
return(value);
}

void set_maxc(char *hi, int v)
{if (v>*hi) *hi=(char)v;}

int  highest_code_in_string(char *p)
{
int i,h;
for (h=i=0;p[i];i++) if (p[i]>h) h=p[i];
return(h);
}


void set_maxc_str(char *hi, char *str)
{set_maxc(hi,strlen(str));}

// Return number of chars needed to store value 'v',
// including one extra if negative
int  digits(double v)
{
long	d;
int i=(v<0);
do	{
	d=(long)(v/=10.0);
	i++;
	} while (d!=0);
return(i);
}

void bit8_set_one_bit(char *mask, int bitnum)
{
BIT8_SET0(mask);
bit_set(mask,bitnum);
}

// If bitmap 'm' has only ONE bit set, return that subscript, else return NOTFND
int bit_only1(char *m, int bit_ct)
{
int i,on=NOTFND;
for (i=0;i<bit_ct;i++)
	if (bit_test(m,i))
		if (on==NOTFND) on=i;
		else return(NOTFND);
return(on);
}

void	bit_set(char *m,int b) {m[b>>3]|=(1<<(b&7));}
void	bit_unset(char *m,int b) {m[b>>3]&=~(1<<(b&7));}
int		bit_test(const char* m, int b) {return((m[b>>3]&(1<<(b&7)))!=0);}

BitMap::BitMap(int _ct)
{bm=(char*)memgive(((ct=_ct)+7)/8);};

BitMap::~BitMap()
{memtake(bm);};

void BitMap::clear_all(void)
{memset(bm,0,(ct+7)/8);};

void BitMap::set(int b)
{bit_set(bm,b);};

void BitMap::unset(int b)
{bit_unset(bm,b);};

int BitMap::is_on(int b)
{return(bit_test(bm, b));};

int BitMap::first(int on_wanted)
{
if (on_wanted) on_wanted=true;
for (int b=0; b<ct ; b++)			// Either we found a 4-byte int containing req'd bit setting, or looking at last 'int'
	if (is_on(b)==on_wanted)
		return(b);	// (if there's a bit with the setting we want, return the subscript)
return(NOTFND);	// If we get here, no bits have the wanted setting
};

int BitMap::Bit_only1(void)
{return(bit_only1(bm,ct));}

QSTR::QSTR(char *ip, int qt)
{
strfmt(str=(char*)memgive(strlen(ip)+3),"%c%s%c",qt,ip,qt);
}

QSTR::~QSTR()
{
memtake(str);
}



// As strancpy(), but discards characters from MIDDLE rather than END if src is too long
char *strancpy_outer(char *dst, const char *src, int bufsz)
{
int len=strlen(src);
if (len<bufsz || bufsz<7) return(strancpy(dst,src,bufsz));
bufsz-=6;				// Allow for "[...]" replacing non-copied chars in middle of o/p, PLUS EOS
int	part1=(bufsz+1)/2;	// (can't be less than 1 char to be copied to 'part1' of output string)
memcpy(dst,src,part1);
strcpy(&dst[part1],"[...]");
return(strcat(dst,&src[len-(bufsz-part1)]));	// (append at least EOS to o/p, more if there's room)
}


#define LASTERROR_SIZE 512	// This is the INTERNAL limit on error text as built up by dll's
// Calling apps get max 255 (254 + EOS), discarding characters from the middle if req'd

static char lastError[LASTERROR_SIZE];
int __stdcall DllGetLastError(LPSTR lpRec)
{
strancpy_outer(lpRec,lastError,255);
//strancpy(lpRec,lastError,250);
//strcpy(lpRec,lastError);
return(0);
}

int __stdcall DllGetVersion(LPSTR lpRec)
{
strcpy(lpRec,__TIMESTAMP__);
return(0);		// 0=No Error
}

static void seterr(const char *fmt, va_list va)
{
_strnfmt(lastError,LASTERROR_SIZE-2,fmt,va);
if (*lastError) strcat(lastError,"\n");
}

void SetErrorText(const char *fmt,...)
{
va_list va;
va_start(va,fmt);
seterr(fmt,va);
}

void SetErrorText_if_blank(const char *fmt,...)
{
if (*lastError==0)
	{
	va_list va;
	va_start(va,fmt);
	seterr(fmt,va);
	}
}

int RetErrorText(int errcode, const char *fmt,...)
{
va_list va;
va_start(va,fmt);
seterr(fmt,va);
return(errcode);
}


extern	int dbsafe;
int		log200;
void m_finish(const char *fmt,...)		// run debugger to HERE if untrapped error
{
va_list va;
va_start(va,fmt);
seterr(fmt,va);
if (log200) sjhLog("ErrorText:%s:%s",caller_name,lastError);
dbsafe=NOTFND;				// Stops dbsafeclose() from marking databases as 'clean' when closing
throw(SE_USER_CANCEL);
}

// This is NOT the same as the ANSI standard strncpy(), which DOESN'T
// put a null-byte at the end of destination string if source is too long
char	*strancpy(char *dst, const char *src, int bufsz)
{
int i;
for (i=0; i<bufsz-1 && src[i]; i++)
	dst[i]=src[i];
dst[i]=0;
return(dst);
}

// Copy from 'src' to 'dst' up to, but not including, the character 'up2',
// or all chars if 'up2' not present. Return 'dst' terminated with a nullbyte
char *stracpy2(char *dst,const char *src, char up2)
{
int i=0,c;
while ((c=src[i])!=0 && c!=up2) dst[i++]=(char)c;
dst[i]=0;
return(dst);
}

char *vb_item(const char *str, int n, char *dst)
{
static int sw;
static char w[4][32];
int	p;
if (!dst) dst=w[sw++&3];
while (n--)
	if ((p=stridxc(TAB,str))==NOTFND) str=strend(str);
	else str+=(p+1);
return(stracpy2(dst,str,TAB));
}


static int next_delimiter(int chr, const char *str)
{
int p;
if ((p=stridxc((char)chr,str))==NOTFND)
	{
	m_finish("Internal error 800 finding CHR(%d)",chr);
	}
return(p);
}

// Return pointer to null-terminated static copy of \t-delimited field 'n' within VB text-format record structure 'rec'
char *vb_field(const char *rec, int n)
{
static char fld[64];
const char *prv=rec;
int len=0;
while (n>=0)
	{
	len=next_delimiter('\t',rec);
	if (len>=sizeof(fld)) m_finish("Internal error 801 field too long [%s]",prv);
	if (n--) rec+=(len+1); else break;
	if (!*rec) return(0);	// Added check to avoid going past end, 7/2/19
	}
if (len) memmove(fld,rec,len);
fld[len]=0;
return(fld);
}


int time_diff(int a, int b)
{
b-=a; a=ABS(b);
if (a>720) {a-=1440; if (a<0) a=-a;}
return(a);
}

char	*hm_str(long dttm)
{
static int i=0;
static char hm[4][6];
return calfmt(hm[i++&3],"%02T%02I",dttm);
}


char	*dmy_str(long bd)			// Consecutively uses one of 4 static areas
{									// for formatting, so one print() statement
static int sw=0;					// can have include up to 4 dates in 1 call
static char dmy[4][9];
return(calfmt(dmy[sw++&3],"%02D%02O%02Y",bd));
}

char	*dmy_hm_str(long dttm)		// Consecutively uses one of 4 static areas
{									// for formatting, so one print() statement
static int sw=0;					// can have include up to 4 dates in 1 call
static char dmy[4][15];
return(strfmt(dmy[sw++&3],"%s %s",dmy_str(dttm),hm_str(dttm)));
}

char	*dmy_hm_dep_str(long dttm)	// Consecutively uses one of 4 static areas
{									// for formatting, so one print() statement
static int sw=0;					// can have include up to 4 dates in 1 call
static char dmy[4][19];
return(strfmt(dmy[sw++&3],"%s.%d",dmy_hm_str(dttm),DTTM2DEP(dttm)));
}


char	*dmy_stri(short bd)
{return(dmy_str(long_date(bd)));}

char	*dmy_stri2(short *bd)		// Format a DateRange from passed 2-element array into static buffer
{
static char s[20];
return(strfmt(s,"%s - %s",dmy_stri(bd[0]),dmy_stri(bd[1])));
}

char e03str2dop(const char *str)	// 
{
int		i,j;
char	c,dop;
for (i=dop=0; (c=str[i])!=0; i++)
	if ((j=stridxc(c,"$MTWHFS"))!=NOTFND)	// Convert MTWHFS$ to bits 1,2,4,...64=Sun,Mon,Tue,...Sat
		dop|=(1<<j);
return(dop);
}

// Our standardised dop bits are 1,2,4,..64 to be Sun,Mon,Tue,...Sat  (Note - bitvalue 128 is never used)
// Build the (non-standard) *.E03 string format from chars mtwhfs$ (in that order), for returning to PM
char *dop2e03str(char dop)
{
static char str[16];
*str=0;
for (int i=1;i<7;i++)
	if (dop&(1<<i)) strendfmt(str,"%c","MTWHFS"[i-1]);
if (dop&1) strcat(str,"$");
return(str);
}


short c2bd(const char *p)				// Return integer Binary Date from DDMMYY string
{return(short_bd(caljoin(a2i(&p[4],2),a2i(&p[2],2),a2i(p,2),0,0,0)));}


int c2bd2(short *bd, const char *from, const char *to)			// Convert 2 string dates to elements in passed array
{
for (int i=0;i<2;i++)
	{
	bd[i]=(short)c2bd(i?to:from);
	if (calerr()) goto fail;
	}
cal_check_date_range(bd[0],bd[1]);
return(NO);			// no error
fail:
return(SE_DATE);
}
void vb_bd2(short *bd, char *str, int sub)
{
bd[0]=c2bd(vb_field(str,sub));
bd[1]=c2bd(vb_field(str,sub+1));
}

#define HEX_CHAR(c) ("0123456789ABCDEF"[c])	// 55
// Format 'nibble_ct' unhexed bytes from 'src' into 'dst', returning 'dst' as a convenience
char *x2c(char *dst, const char *src, int nibble_ct)
{
static char w[16];
int ct, i, nibble, chr;
if (!dst && nibble_ct<=sizeof(w)) dst=w;
ct=i=chr=0;
while (ct<nibble_ct)
	{
	if (ct&1) nibble=chr&15; else nibble=(chr=src[i++])>>4;
	dst[ct++] = HEX_CHAR(nibble);
	}
return(dst);
}

ulong h2l(char *s, int nibble_ct)
{
int i, nibble;
ulong n;
for (n=i=0;i<nibble_ct;i++)
	{
	nibble=s[i]-'0';
	if (nibble<0 || nibble>15) m_finish("Bad hex value [%c]",s[i]);
	n=((n<<4)|nibble);
	}
return(n);
}



int	x2bd(const char *p)
{
int i, n[3];
for (i=0;i<3;i++)
	{
	n[i]=x2l(p, (short)(i*2), 2);
	if (a2err) return(0);
	}
i=short_bd(caljoin(n[2],n[1],n[0],0,0,0));
return(calerr()?0:i);
}

// Format binary date for VB (cccc-mm-dd) into passed buffer 'vb'
// Return 'vb' as a convenience
// If 'vb' isn't passed, use our own local static buffer
char *bd2vb(short bd, char *vb)
{
static char dummy[16];
if (!vb) vb=dummy;
calfmt(vb,"%04C-%02O-%02D",long_date(bd));
return(vb);
}

short vb2bd(const char *vb)
{
int d,m,y;
if ( vb[4]!='-' || vb[7]!='-'
||	(y=a2i(vb,4))<1980 || a2err
||  (m=a2i(&vb[5],2))<1 || a2err
||  (d=a2i(&vb[8],2))<1 || a2err
||  (d=short_bd(caljoin(y,m,d,0,0,0)))<1 || calerr())
	d=0;
return((short)d);
}

void bd2x(char *p, int bd)
{l2x(p,0,6,a2l(dmy_stri((short)bd),6));}
char	*hm_stri(int m)	// return hhmm string from binary mins
{									// flag & 1 = use ':' as separator
static int sw;						// flag & 2 = space in 1st char if 0
static char hm[4][6];
return(strfmt(hm[sw++&3],"%02d%02d",(m%1440)/60,m%60));
}


void hm2x(char *p, int hm)
{l2x(p,0,4,a2l(hm_stri(hm),4));}

/*
// Find the offset in addr[] of ANY of the characters in string srch[]
// (Maximum 'len' characters from addr[] to be checked)
int memidxs(char* srch, char* addr,int len)
{
for (int p=0;p<len;p++) if (stridxc(addr[p],srch)!=NOTFND) return(p);
return(NOTFND);
}
*/

// Return 1-32 char string of flag bits, each 0/1
char *bit2c(int flag, int numbits)
{
static int	sw=0;						// delivers up to 4 strings with different addresses
static char _s[4][33];
char *s=_s[sw++&3];
s[numbits]=0;							// EOS byte
for (int i=0;i<numbits;i++) s[numbits-1-i]=(char)('0' + ((flag>>i)&1));
return(s);
}


char *strcpy_trim(char *op, const char *ip, int maxlen)
{
int i, pos, c;
for (i=pos=0;i<maxlen;i++)
	if ((c=ip[i])>' ' || pos)
		op[pos++]=(char)c;
do op[pos--]=0;
	while (pos>=0 && op[pos]<=' ');
return(op);
}


char *unquote(char *t)
{
int	q;
while ((q=stridxc(34,t))!=NOTFND) strdel(&t[q],1);
return(t);
}

// Return pointer to element 'n' in comma-separated list, or an empty string if there aren't that many elements
const char *str_field(const char *str, int n)	// NOTE - n=1 for the first item!
{
int	comma;
const char	*s;
for (s=str;--n;s=&s[comma+1])
	if ((comma=stridxc(COMMA,s))==NOTFND) return("");
return(s);
}


// These are fairly common 64 Kb "formatting callback" requirements, so we just have one copy of each ( fmt_ )
void fmt_str(const void *str, char *vb) {strfmt(vb,"%s\t\n",str);}
void fmt_bd(const short *bd, char *s) {strfmt(s,"%s\t\n",dmy_stri(*bd));}


// get/put _binary124 use Intel "backwords" native binary storage mode
ulong get_binary124(const void *addr, int len, int sub)
{
switch (len)
	{
	case 1: return(((char*)addr)[sub]);
	case 2: return(((ushort*)addr)[sub]);
	default:return(((long*)addr)[sub]);
	}
}

void put_binary124(void *addr, ulong value, int len, int sub)
{
switch (len)
	{
	case 1: ((char*)addr)[sub]=(char)value; break;
	case 2: ((ushort*)addr)[sub]=(ushort)value; break;
	default: ((ulong*)addr)[sub]=value;
	}
}

ushort r2i(const char *s)
{
return((ushort) ((((ushort)s[0])<<8)|s[1]));
}

short *r2i2(short *i, const char *r)
{
i[0]=r2i(r);
i[1]=r2i(&r[2]);
return(i);
}


ulong r2l(const char *s)
{
return ((((long)s[0])<< 24) | (((long)s[1])<< 16) | (((short)s[2])<<8) | s[3]);
}

ulong r2long(char *ad, int len)
{
switch (len)
	{
	case 1: return(*ad);
	case 2: return(r2i(ad));
	default:return(r2l(ad));
	}
}


ulong a2l(const char *s,int ct)
{
int i,c;
ulong result;
result = i = 0;
if (!ct) ct = strlen(s);
while (s[i]==' ' && ct) {ct--;i++;}
a2err = 2; a2err_char=0;
while (s[i] && ct--)
	{
	if ((a2err = !(((c = s[i++]-'0') >= 0 ) && (c <=9 ))) == 0)
		result = result*10 + c;
	else
		{
		a2err_char=c+'0';
		break;
		}
	}
return(result);
}

int a2l_signed(const char *str, int len)
{
if (str[0]=='-')
	return(-(int)a2l(&str[1],len?(len-1):0));
return(a2l(str,len));
}

ushort a2i(const char *s,int ct){return (ushort)a2l(s,ct);}

char *l2r(ulong n)
{
static char c[4];
c[0] = (char)(n >> 24);
c[1] = (char)(n >> 16);
c[2] = (char)(n >> 8);
c[3] = (char)n ;
return c;
}

char *i2r(ushort n)
{
static char c[2];
c[0] = (char)(n >> 8);
c[1] = (char)n ;
return c;
}

// 's' is the character representation of a number that may end with '.' and
// up to 2 decimal digits. Return binary 'value in pence' (if 's' is actually
// a module name the return value is the module file id).
ulong a2lx100(const char *s)	// Conversion function
{
ulong v=a2l(s,7)*100L;
int	d=stridxc('.',s);
if (d!=NOTFND) v+=a2i(&s[d+1],2);
return(v);
}

char *lx1002c(ulong v)	// Conversion function
{
static	char s[32];
ulong	i;
strfmt(s,"%07lu",v/100L);
if ((i=(v%100L))!=0) strendfmt(s,".%u",i);	// MODFILEEXT
return(s);
}
	

// Format (long) 'ctl' value as text-format Disc-ID "ddmmyy hhmm.d", returning pointer to local static text buffer
char	*ctl2c(long ctl)	// Conversion function
{
static int sw;
static char s4[4][32];
char *s=s4[sw++ % 3];
strfmt(s,"%s %s",bd2vb(short_bd(ctl)),hm_str(ctl));
//if (DTTM2DEP(ctl)) 
	strendfmt(s,".%-2.2d",DTTM2DEP(ctl));
return(s);
}

// Return (long) 'ctl' value of text-format Disc-ID "ccyy-mm-dd hhmm.d"
long	c2ctl(const char *s)
{
int		p=stridxc(' ',s);
long	ctl = vb2bd(s)*ONE_DAY + (a2hm(&s[p+1])%1440)*60L;
if ((p=stridxc('.',s))!=NOTFND)
	{
	int seconds_or_depot=a2i(&s[p+1],0);
	if (seconds_or_depot>=60) ctl=0; else
	ctl+=seconds_or_depot;
	}
return(ctl);
}

char	*ctlmod2c(long ctl, long mod)
{
static int sw;
static char s4[4][48];
char *s=s4[sw++ % 3];
strfmt(s,"Disc:%s  File:%s",ctl2c(ctl),lx1002c(mod));
return(s);
}



char *_strfmt(char*s1 ,const char*s2,va_list va)
{
static char w[512];
if (!s1) s1=w;
vsprintf(s1,s2,va);
return s1;
}

char *_strnfmt(char*s1,int n, const char*s2, va_list va)
{
static char w[512];
if (!s1) s1=w;
_vsnprintf(s1,n,s2,va);
return s1;
}

char *strfmt(char *str ,const char *fmt,...)
{
va_list va;
va_start(va,fmt);
return _strfmt(str,fmt,va);
}

char *strnfmt(char *str, int n, const char *fmt,...)
{
va_list va;
va_start(va,fmt);
return _strnfmt(str,n,fmt,va);
}


//, there's a problem if the appended item is > 64 bytes long!!!
void vb_append(char *vbuf, const char*fmt,...)
{
va_list va;
va_start(va,fmt);
char wrk[80];
if (*vbuf) strfmt(wrk,"%c",TAB); else *wrk=0;
_strnfmt(strend(wrk),64,fmt,va);
if (strlen(vbuf)+strlen(wrk)+1>65000)
	throw SE_BUFF64K;
strcat(vbuf,wrk);
//_strnfmt(strend(vbuf),64,fmt,va);
}

int stridxc(char c,const char *s)
{
const char *p=strchr(s,c);
return(p?p-s:NOTFND);
}

char *strend(const char *s)
{
return (char *)&s[strlen(s)];
}


char *stradup(const char *s)
{
return (strcpy((char*)memgive(strlen(s)+1),s));
}

char *strinsc(char *str,int chr)	// Insert chr at start of str
{
memmove(str+1,str,(strlen(str)+1));
*str=(char)chr;
return(str);
}

int strinspn(const char *sub, const char *str)
{
int i;
for(i = 0; str[i];i++)	
	if(stridxc(str[i],sub)!=NOTFND)
		return i;
return NOTFND;
}

char *strins(char *str, const char *ins)	// Insert ins at start of str
{
int	inslen=strlen(ins);	/* length of insert string */
if	(inslen > 0) 
	{
	memmove(str+inslen,str,(strlen(str)+1));/* move original */
	/* moved an extra byte to make sure 3rd arg non-zero */
	memmove(str,ins,inslen); /* move the insert */
	}
return(str);
}

/*	Inserts a sub-string at the beginning of a string. If the new length would exceed the Buffer size (strlen()+1),
	the string is truncated before insertion. Returns the input string address
char	*strnins(char *str,const char *ins,int maxlen)
{			// Insert ins_str at start of str, truncate if reqd
int	inslen, curlen;
maxlen--;				// exclude final end of string
inslen=strlen(ins);	// length of insert string
if	(maxlen <= 0 || inslen <= 0) goto end; // nothing to be done
if	(inslen>=maxlen) inslen=maxlen;	// partfit insert only - trunc insert
else				// some of the original STR fits
	{
	curlen = strlen (str);				// size of original
	if	(inslen+curlen>maxlen) curlen=maxlen-inslen; // not both - trunc original
	else maxlen=inslen+curlen;			// both - mark where terminating NULL goes
	memmove(str+inslen,str,(curlen+1));	// move original
	// moved an extra byte to make sure 3rd arg non-zero
	}
memmove(str,ins,inslen);	// move the insert
*(str+maxlen) = 0;		// put in final end of string
end:
return(str);
}
*/

// why not test the whole of 'sub'? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int	stridxs(const char *sub, const char *str)	// Find offset of sub in str
{
int	offset,sp=0;
while	( (offset=stridxc(*sub,&str[sp])) != NOTFND )
	if (!memcmp(&sub[1],&str[sp+=offset+1],strlen(&sub[1]))) return(sp-1);
return(NOTFND);											// 0 or >0 if rest matches
}

char *strdel(char *str, int ct)
{
int len=strlen(str);
if (ct>len) ct=len;
return (char *)(memmove(str,&str[ct],len-ct+1));
}


void strendfmt(char *s, const char *ctl,...)
{va_list va; va_start(va,ctl); _strfmt(strend(s),ctl,va);}

static char *strpwhit(char* str)
{
while((*str == '/t')||(*str == ' '))str++;
return str;
}

static char *strrtrim(char *s)
{
int i,c;
for (i=strlen(s);--i>=0;s[i]=0)
	if ((c=s[i])!=TAB && c!=SPACE)
		break;
return s;
}

char *strtrim(char *str)				// Remove leading & trailing white space
{
char *sss=strrtrim(strpwhit(str));
memmove(str, sss, strlen(sss)+1 );
return(str);
}

/*
char *strpad(char *s, int wid)	// Pad 's'to 'wid' with trailing spaces
{								// DON'T TRUNCATE IF LONGER!!! (16/11/98)
int i;
for (i=0;i<wid;i++) if (!s[i]) *(short*)&s[i]=' ';
//s[i]=0;
return(s);
}
*/

/*int strnicmp(const char* s1, const char* s2, int n)	// Any difference?
{													// (incl. one shorter)
while (n--) if (TOUPPER(s1[n])!=TOUPPER(s2[n])) return(YES);
return(NO);
}*/

ulong hex2l(const char *s, int len)
{
int i, n;
for (i=n=0;i<len;i++)
	{
	int c=TOUPPER(s[i]);
	if (ISUPPER(c)) c=10+(c-'A');
	else if (ISDIGIT(c)) c-='0';
	else return(0xFFFFFFFF);
	n=(n<<4)+c;
	}
return(n);
}



static void addnib(ulong &t,char vv, int hi)
{
int v;
if (hi) v=vv>>4; else v=vv&15;
if (v>=10) {a2err=1; a2err_char=vv;}
else t = t * 10 + v;
}

long x2l(const char *byte_addr, short nib_offset, short nib_len)
{
ulong result=0;
a2err=0;
while (nib_offset>=2) {byte_addr++; nib_offset-=2;}
if (nib_offset) {addnib(result,*(byte_addr++),NO); nib_len--;}
while (!a2err && nib_len>0)
	{
	char v=*(byte_addr++);
	addnib(result,v,YES);
	if (--nib_len) addnib(result,v,NO);
	nib_len--;
	}
return((long)result);
}

void l2x(char *byte_addr, short nib_offset, short nib_len, long val)
{
while (nib_offset>=2) {byte_addr++; nib_offset-=2;}
ulong v=(ulong) val;
for (int hi_nib=((nib_len + nib_offset)&1); nib_len>0; hi_nib=!hi_nib)
	{
	char c, *a=&byte_addr[(--nib_len + nib_offset)/2];
	if (v) {c=v%10;	v/=10;} else c=0;
	if (hi_nib) *a = ((*a)&0x0F) | (c*16);
	else *a = ((*a)&0xF0) | c;
	}
}

short	memidxw(short val,const short *ary,short tsize)
{
for (int i=0; i<tsize; i++) if (ary[i]==val) return(i);
return(NOTFND);
}


#ifdef pre_100909
long unused_x2l(const char *byte_addr, short nib_offset, short nib_len)
{
char error;
long result;
_asm{
;		Convert 'nib_len' nibbles of Hex-encoded data at offset 'nib_offset'
;		from 'byte_addr' into a long value to be returned (offset is usually
;		either 0 or 1, but it can be any value that suits the calling prog).
;		The main loop selects only one nibble at a time (toggles hi or lo),
;		so it only increments the input address pointer on alternate passes.
;		The only tricky bit is getting 'ch' correctly set before the loop
;		start, so we know if we start on the hi or lo nibble on first pass.
;		The other point is that I've used a fiddly way to multiply edx by 10,
;		which is faster than 'imul', but doesn't compile into bigger code.
;		Finally, it turns out that 'dec cl / jnz agen' is noticeably faster
;		than using 'loop agen', which we could have done (but this way is also
;		better because we can use 'ch' to hold the 'current nibble number',
;		which means that 'bx' is not used at all in this function).

		mov	esi,byte_addr	; get base addr
		mov	ax,nib_offset	; get NIBBLE offset
		mov	cx,nib_len		; get input len in cl, leaving ch clear
		shr	ax,1			; halve the offset
		adc	ch,0			; The LSB from 'offset' == 'Nibble No.'
		add	si,ax			; add any whole bytes in 'offset' to address
		xor	edx,edx			; initialise output
		mov	error,0
agenx:
		xor	eax,eax			; clear the high-order parts of eax
		mov	al,[esi]		; get the next char into lo-byte
		mov ah,0
		xor	ch,1			; toggle 'Nibble No.' between 0 and 1
		jz	nib2			; and jump forward if we're doing 2nd nibble
		shr	al,4			; (here, we're doing the 1st nibble of this byte)
		jmp	skip
nib2:
		and	al,15			; Here we're on the 2nd nibble, so strip hi-nibble
		inc	si				; and increment address offset for next byte
skip:

	cmp al,10
	jl ok
	mov error,al
ok:
		shl	edx,1			; double existing value
		add	eax,edx			; add the doubled value to current char giving 'i'
		shl	edx,2			; multiply the doubled value by 4, giving 8 x Org
		add	edx,eax			; add 'i' from above. 		<- Gives edx=edx*10+al
		dec	cl				; Any more chars?			but much faster, and
		jnz	agenx			; YES - round again			no bigger than imul
		mov	al,error
		and	al,al
		jz ok1
		xor edx,edx
ok1:
		
		mov result,	edx
		
	};
a2err=error!=0;
a2err_char=error;
return result;
}


void Al2x(char *byte_addr, short nib_offset, short nib_len, long val)
{
int ten=10;
_asm{
;	//	This is the reverse of x2l(), but it turns out to be a bit harder to
;	//	implement. I couldn't see an easy way to avoid using 'idiv', but this
;	//	routine is less likely to be time-critical, so never mind. The tricky
;	//	bit here is getting the address to point to the END of the output area
;	//	(with the 'current nibble' toggle appropriately set), so I can move
;	//	backwards setting output nibbles to 'value MOD 10', and dividing the
;	//	value by ten after each pass. Each pass is either going to output the
;	//	1st or 2nd nibble at the current address. I couldn't avoid duplicating
;	//	a couple of instructions to actually do this, 'cos one pass has to
;	//	increment the address afterwards (the lo-nibble pass doesn't, since
;	//	it'll be doing hi-nibble at the same address on the next pass).
		mov	esi,byte_addr	;// get base addr
		mov	ax,nib_offset	;// get NIBBLE offset
		mov	cx,nib_len		;// get input len in cl, leaving ch clear
		shr	al,1		;// halve the nibble offset to give whole bytes
		adc	ch,0		;// Ends up with ch holding LSB of 'offset'
		add	si,ax		;// add any whole bytes in 'offset' to address
		mov	al,cl		;// ds/si address currently points to output area START,
		cmp	ch,1		;// with 'ch' telling us if it's nibble 0 or 1. These
		jz	odd			;// lines will advance 'si' so that we're pointing to
		sub	al,1		;// the END of the output ('ch' adjusted as appropriate).
odd:	mov	ch,al		;// When this has been done, we'll process right to left
		and	ch,1		;// instead of left to right, 'cos it's easier to work
		shr	al,1		;// out the next digit on each pass using 'idiv'.
		add	si,ax		;// (si now points to the RIGHTMOST byte to be written to)
		mov eax,val		;// get the (long) value to be converted
rptx:	xor	edx,edx		;// clear the remainder field
		idiv DWORD PTR ten	;// divide eax by 10, remainder in edx
		mov	bl, [esi]	;// get the existing char at this output address
		xor	ch,1		;// toggle 'Nibble No.' - alternately 1st/2nd
		jz	nb2			;// If ch==0, jump to process 2nd nibble
		and	bl,0Fh		;// Doing 1st nibble, so preserve 2nd nibble of existing
		shl	dl,4		;// byte (as just loaded into bl), and shift the next
		or	dl,bl		;// digit (as just got in dl) into the hi-nibble position
		mov	[esi],dl	;// re-save the new byte
		dec	si			;// As this is the first nibble, we move back
		jmp	skp			;// one more byte now on the output address
nb2:	and	bl,0F0h		;// 2nd nibble - replace the lo-nibble
		or	dl,bl		;// of the byte with the next digit
		mov	[esi],dl
skp:	dec	cl			;// any more digits to process?
		jnz	rptx		;// YES - go and do another
	}
}

short	Amemidxw(short val,const short *ary,short tsize)
{
short result=-1;
_asm{
		; **************** Next line prevents table overrun ******************
	;	mov ecx,0
		; **************** Prev line prevents table overrun ******************
		movzx	ecx, tsize		;Load table length	; MUST BE MOVZX TO KEEP HI-BYTES ZEROISED!!!
		cld					;set direction flag
		mov	edi,ary			;Load ptr to table
		mov	esi,edi			;save addr
		mov	ax,val			;Load value to scan for
	repnz	scasw			;scan to table end
		mov result,NOTFND	;default return
		jnz	endch			;no match
		sub	edi,esi			;get table width scanned to find item
		shr	edi,1			;halve it to give subscript 1...MX
		dec	edi				;need it as 0...MX-1
		mov result,di
		endch:
	};
return result;
}
#endif  // of suspect asm routines replaced by C++ equivalents 100909


// Create a null-terminated string with all chars set to 'fillchr' 
char	*strfill(char *str,int width,int fillchr)
{
str[width]=0;
while (width--)
	str[width]=(char)fillchr;
return(str);
}

// left/right/centre justify a string in situ, using 'chr' to pad as required
char	*strjust(char *s,int wid,int typ,int chr)	// TRUNCATE THE STRING IF IT'S LONGER THAN 'WIDTH'
{
int sw;
for (sw=s[wid]=0;strlen(s)<(unsigned)wid;sw=!sw)
	if (typ==STR_LJUST || (typ==STR_CENTER && sw)) strcat(s,(char*)&chr);
	else strinsc(s,chr);
return(s);
}

///* right-justify with prepended blanks			// DOESN'T TRUNCATE THE STRING IF IT'S LONGER THAN 'WIDTH'
char	*strrjust(char *str, int width)
{
while (strlen(str)<(unsigned)width) strinsc(str,' ');
return (str);
}
//*/

/*	Overwrite a string with another string.  The length of the destination string is never changed - the overwrite
	halts when either string is exhausted. Returns the input string address
char	*strover(char *str, char *ovr)
{
int	i;
for (i=0;str[i] && ovr[i];i++) str[i]=ovr[i];
return(str);
}
*/


TIMER::TIMER(int delay_csecs, int initial_delay) {iv=initial_delay;delay=delay_csecs;tmr=tmrist(0);}
//TIMER::TIMER(void) {tmr=tmrist(iv=0);}
TIMER::TIMER(void) {delay=100;tmr=tmrist(iv=0);}

TIMER::~TIMER() {tmrrls(tmr);}

void __stdcall Xecho(const char *fmt,...);
int TIMER::tick(void)
{
if (tmrelapsed(tmr)<iv) return(NO); 
if (iv<delay && (iv+=(iv+5)/5)>delay) iv=delay;
tmrreset(tmr,0); return(YES);
}

void TIMER::log(char *txt)
{
sjhLog("T=%ld  %s",tmrelapsed(tmr),txt);
}
