#include "xlib.h"
#include "dll.h"
#include <io.h>
#include "ti_var.h"


char *sjhtxt(char *buf, char *mode)
{
short m;
if ((m=*mode)=='r') m='R';
HDL f=flopen_trap("sjh.txt",(char*)&m);
if (m=='R')
	{
	int len=flget(buf,65535,f);
	buf[len]=0;
	}
else
	flput(buf,strlen(buf)+1,f);
flclose(f);
return(buf);
}


LOGTIMER::LOGTIMER(char *_txt)
{
txt=stradup(_txt); tmr=tmrist(0);
}

LOGTIMER::~LOGTIMER()
{
sjhLog("%s %0.2f",txt,0.01*tmrelapsed(tmr));tmrrls(tmr);
memtake(txt);
}

void sjhlog(char *fmt,...)
{
if (!fmt) {logpath[0]=0;return;}
char w[1024];
if (!logpath[0])
	{
	strcpy(logpath,search_path0());
	extern int	dlllog;
	if (dlllog==YES)
		{
//?//		GetTranscendRegistry(w,"LogPath");
		strcpy(w,"\\log");
		if (*w && drisdir(drfullpath(w,w))) strcpy(logpath,w);
		}
	strcat(logpath,"\\SJH.LOG");
	if (!fmt[0]) return;						// If first call would just write blank line don't bother
	}
va_list va; va_start(va,fmt);
_strfmt(w,fmt,va);

extern int flopen_status;
flopen_status=2;
HDL f=flopen(logpath,"a");
if (!f) throw(SE_SJHLOG);
flputln(w,f);
flclose(f);
}

void sjhlog_hex(char *p, int sz)
{
char s[128];
int	lsz;
for (int i=0;i<sz;i+=lsz)
	{
	s[0]=0;
	lsz=sz-i;
	if (lsz > 16) lsz=16;
	for (int j=0;j<lsz;j++)
		strendfmt(s,"%02X ",p[i+j]);
	sjhloG("%s",s);
	}
sjhloG(".");
}
	

// MUST call this immediately within dllclosedown, because fn in calling app has prolly gone out of scope!
void clear_callbacks(void)
{
DllRegisterStatusFn(0);
DllRegisterCancelFn(0);
}


char *ti_var(const char *varnam, char *contents, int opt)
{
return(tiv->get(varnam,contents,opt));
}


// Return integer value of TI-variable 'varnam', or default if n/f
int ti_vari(const char *varnam, int deflt)
{
char	w[32];
if (*ti_var(varnam,w,0))
	{int i=a2l(w,0); if (!a2err) deflt=i;}
return(deflt);
}

// Return TRUE if TI-variable 'varnam' starts with Y or y
int ti_var_y(const char *varnam)
{
char	w[128];
return(*ti_var(varnam,w,TIV_UPPER)=='Y');
}

char*	ti_filespec(char *pth, const char *fn)
{
strendfmt(drfullpath(pth,ti_var(fn,pth,TIV_PATH)),"\\%s",fn);
return(pth);
}

static int comma2null(char *str)	// change comma-separators to NULL's
{
int q=strlen(str), ct, p;
if (q && str[q-1]!=COMMA) *(short*)&str[q]=COMMA;
for (ct=p=0; (q=stridxc(COMMA,&str[p]))!=NOTFND; ct++)
	{
	str[q]=TAB;
	p+=(q+1);
	}
return(ct);
}


// If (bitflag & TIV_MULTI), value[0] == INPUT parameter specifying max number of tab-separated "strings" to return.
int __stdcall DllGetSetting(LPSTR name, LPSTR value, int bitflag)
{
int err=0;
try	{
	char *p=value;
	int i, mx=value[0];
	int ct=*ti_var(name,value,bitflag);
	if (bitflag&(TIV_MULTI|TIV_TAB))	// Returning multistrings - change \0 delimiters to TAB's,
		{								// and append empty strings for missing entries at the end of the list
		if (bitflag&TIV_TAB) ct=comma2null(value);	// (change comma-separators to \0's, prior to making them TAB's)
		DYNAG d(0);
		for (i=0;ct--;i+=(strlen(&value[i])+1))
			d.put(&value[i]);
		while (d.ct<mx) d.put("");
		for (i=value[0]=0;i<d.ct;i++)
			{
			if (i) strendfmt(value,"%c",TAB);
			strendfmt(value,(char*)d.get(i));
			}
		}
	}
catch (int errcode)
	{
	err=errcode;
	}
//Sjhlog("name=%s value=%s flag=%d err=%d",name,value,bitflag,err);
return(err);		// 0=No error
}

/*
#define K_Etm 0			// Subscripts for the 5 basic InsKey fields, etc.
#define K_Veh 1			// These are the types for 4-byte expand/compress
#define K_DVR 2
#define K_DTY 3
#define K_MOD 4
*/

ulong encode(char fld, const char *c)
{
ulong m;
int   i,ch;
if (fld==IX_DRIVER || fld==IX_MODULE) return(a2l(c,7-(fld==IX_MODULE)));
for (i=0;i<6 && c[i];i++) if (c[i]>'0') break;
for (m=0;i<6 && (ch=c[i])!=0 && strancmp(&c[i],"      ",6-i);i++)
	{
	m*=36;
	if (ISDIGIT(ch)) m+=(ch-'0');
	else if (ISUPPER(ch)) m+=(ch-'A'+10);
	else return(0);
	}
return(m);
}


char  *expand(char fld,ulong ky)	// returns key in one of 4 static areas,
{									// so we can call more than once from fn()
static	int sw=0;
static   char w[4][8];
int   i,c;
char	*t=w[sw++&3];
if (!ky || fld==IX_DRIVER || fld==IX_MODULE) return(strfmt(t,"%ld",ky));
for (t[6]=i=0;i<6;i++) {c=ky%36; t[5-i]=(c+((c>9)?('A'-10):'0')); ky/=36;}
while (t[1] && t[0]<'1') memmove(t,&t[1],7);
return(t);
}

