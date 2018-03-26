#include "xlib.h"
#include "dll.h"
#include <conio.h>
#include <stdio.h>
#include <direct.h>

int Xstridxc(char ch,const char *str) {return(stridxc(ch,str));}
char *Xstrinsc(char *str,int chr) {return(strinsc(str,chr));}
char *Xstrdel(char *str, int ct) {return(strdel(str,ct));}
HDL Xflopen(const char *fnam, const char *mode) {return(flopen(fnam,mode));}

int Xfleof(HDL h_fl) {return(fleof(h_fl));}

void Xflputln(const void *s, HDL h_fl) {flputln(s,h_fl);}
int Xflgetln(void *s, int maxlen, HDL h_fl) {return flgetln((char*)s, maxlen, h_fl);}
int Xflget(void *s, int maxbytes, HDL h_fl) {return flget((char*)s, maxbytes, h_fl);}

void Xflclose(HDL h_fl) {flclose(h_fl);}

char *Xfload(char *fn, char *buff)
{
HDL f=flopen(fn,"R+");
*buff=0;
while (!fleof(f))
	{
	flgetln(strend(buff),256,f);
	strcat(buff,"\r\n");
	}
int len=strlen(buff);
return(buff);
}


char *Xstrins(char *str, const char *ins) {return(strins(str,ins));}

char *Xstrend(const char *str) {return(strend(str));}
char *Xstrupr(char *str) {return(_strupr(str));}
int Xvb2bd(const char *bd) {return(vb2bd(bd));}
int Xc2bd(const char *bd) {return(c2bd(bd));}

char	*Xdmy_stri(short bd) {return(dmy_stri(bd));}

void Xmemchk(char *s) {memchk(s);}

void Xstrendfmt(char *s, const char *ctl,...)
{va_list va; va_start(va,ctl); _strfmt(strend(s),ctl,va);}


int	Xtimer(int tmr)
{
if (tmr) {int elapsed=tmrelapsed((HDL)tmr); tmrrls((HDL)tmr); return(elapsed);}
return((int)tmrist(0));
}

int	 Xa2i(const char *str, int len) {return(a2l(str,len));}

char *Xstrfmt(char *str, const char *ctl,...)
{
va_list va;
va_start(va,ctl);
return _strfmt(str,ctl,va);
}

void Xsjhlog(char *str) {sjhloG("%s",str);}


// *bigbuf is an (initially null) pointer to a memory block that can become larger than 64 Kb
// If we're called with *smallbuf!=\0, (re-)allocate *bigbuf, append smallbuf string to it, and return YES
// If *smallbuf==\0, return NO
// If smallbuf==NULL, release *bigbuf if it's been allocated
int Xbigbuf(char **bigbuf, const char *smallbuf)
{
if (!smallbuf) {if (*bigbuf) memtake(*bigbuf); return(NO);}
int len=strlen(smallbuf);
if (!len) return(NO);
if (*bigbuf)
	{
	int prv=strlen(*bigbuf);
	strcpy(&(*bigbuf=(char*)memrealloc(*bigbuf,prv+len+1))[prv],smallbuf);
	}
else *bigbuf=stradup(smallbuf);
return(YES);
}
char	*Xvb_field(const char *rec, int n) {return(vb_field(rec,n));}

void __stdcall Xwait(const char *txt)
{
while (_kbhit()!=0) _getch();	// added 250208
printf("%s",txt);
while (_kbhit()==0) {;}
}

int __stdcall Xkbstr(char *ip, int len, int flag)
{
int c,p=0;
while (_kbhit()!=0) _getch();
//c=_getch();
while ((c=_getch())!='\r' || !len)
	{
	if (c==8 && p>0) {p--;printf("\b \b");}
	if ((flag&TIV_UPPER)!=0) c=TOUPPER(c);
	if (!len)
		return(c);
	if (c==ESC) {*ip=0;return(ESC);}
	if (p<len)
		{
		if (ISALPHA(c)) printf("%c",ip[p++]=c);
		}
	}
return(ip[p]=0);
}


void __stdcall Xgetcwd(char *cwd)
{
char zz[FNAMSIZ];
if (!cwd)
	cwd=zz;
_getcwd(cwd,FNAMSIZ-1);
}

void __stdcall Xchdir(char *pth)
{_chdir(pth);}



static void append_queries(char *pth)
{
char fn[FNAMSIZ];
int i=stridxc(SEMICOLON,pth);
if (i==NOTFND) i=strlen(pth);
memmove(fn,pth,i);
fn[i]=0;
i=strlen(drfulldir(fn,fn));	// force terminating backslash, and set i to the following nullbyte
while (i--)
	if (fn[i]=='\\')
		{
		strcpy(&fn[i+1],"Queries");
		if (drisdir(fn))
			{strendfmt(pth,";%s",fn);break;}
		}
}


int __stdcall XCancelCallback(void)
{
if (_kbhit()!=0 && _getch()==ESC) return(YES);
return(NO);
}

static char *Zstracpy2(char *dst,const char *src, char up2)
{
int i=0,c;
while ((c=src[i])!=0 && c!=up2) dst[i++]=c;
dst[i]=0;
strtrim(dst);
return(dst);
}

char *Xvb_item(const char *str, int n, char *dst)
{
static char w[32];
int	p;
if (!dst) dst=w;
while (n--)
	{
	if ((p=Xstridxc(TAB,str))==NOTFND && (p=Xstridxc(LNFEED,str))==NOTFND) {str=Xstrend(str); break;}
	str+=(p+1);
	}
if (str[0]==LNFEED) str++;
return(Zstracpy2(dst,str,TAB));
}


char *Xfieldname(int typ, char *nam)
{
char inf[200];
if (DllGetFieldInfo(typ,inf)) *nam=0;			//, ???? how would this be possible???
Zstracpy2(nam, inf, TAB);
return(nam);
}


char *Xsjhtxt(char *buff, char *mode) {return(sjhtxt(buff,mode));}

int Xlist(char *p, char *txt, int loop)
{
static int ct=0;
int more, tpos, terminated, told=NO;
for (more=loop && p[0]; p[0]; p+=(tpos+terminated))
	{
	char wrk[1024+1];
	tpos=Xstridxc(LNFEED,p);
	terminated = (tpos!=NOTFND && tpos<=1024);
	if (!terminated)
		{
		if (!told++) Xecho("ERROR! ***** No terminating <LF> (Listing as 128-byte blocks)\r\n");
		tpos=strlen(p);
		if (tpos>128) tpos=128;
		}
	memmove(wrk,p,tpos);
	wrk[tpos]=0;
	Xecho("%s\r\n",wrk);
	ct++;
	}
if (!more)
	{Xecho("%d %s Listed\n",ct,txt); ct=0;}
return(more);
}

static int  input_number(char *prompt, int hi)
{
char	s[4];
int ch=0, ln, len=digits(hi);
Xecho("%s:",prompt);
for (ln=0;ln<len;ln++) printf(".");
while (ln) {printf("%c",BKSP); ln--;}
while (YES)
	switch (ch=_getch())
		{
		case BKSP:
			if (ln) {printf("%c.%c",BKSP,BKSP); ln--;}
			break;
		case ESC:
			Xecho("<ESC>"); return(NOTFND);
		case CRET:
			if (ln && (ch=a2i(s,ln))<=hi)
				{
				while (ln) {printf("%c.%c",BKSP,BKSP); ln--;}
				Xecho("%d",ch);
				return(ch);
				}
			break;
		default:
			if (ISDIGIT(ch) && ln<len) printf("%c",s[ln++]=ch);
			break;
		}
}

int Xlist_and_pick(char *bb, char *name)
{
if (!bb || !*bb) return(-1);
Xlist(bb,name,0);
int		ct;
for (ct=0;*bb;ct++) bb=&bb[Xstridxc(LNFEED,bb)+1];
ct=input_number("Enter required subscript",ct-1);
printf("\n");
return(ct);
}

void Xlist_fields(char *p, char *txt)
{
int ct;
for (ct=0;*p && *p!=LNFEED;ct++)
	{
	char *str=vb_field(p,0);
	Xecho("%s\n",str);
	p+=(strlen(str)+1);			// step over the field contents AND the tab that should follow
	}
Xecho("%d %s Listed\n",ct,txt);
}

