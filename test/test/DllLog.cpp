#include "xlib.h"
#include "DLL.h"
#include "sys/types.h"
#include "sys/timeb.h"
#include "time.h"

int		dlllog=NOTFND, logopt;
char	logpath[FNAMSIZ];

static	DYNAG	*nolog;

void nolog_rls(void)
{ SCRAP(nolog); }

int nolog_this_dll(void)
{
if (nolog)
	{
	char w[48];
	if (nolog->in( _strupr(strcpy(w,caller_name)) )!=NOTFND) return(YES);		// ENTIRE DLL is explicitly set for no Logging
	}
return(NO);
}

static int no_log(char *fun)
{
if (!nolog) return(NO);						// DON'T ignore if no exceptions are defined, because we obviously want to log this call
if (nolog->in(fun)!=NOTFND) return(YES);	// If it's specified, DO ignore it in calling routine
char w[48];
_strupr(strfmt(w,"%s:%s",caller_name,fun));
if (nolog->in(w)!=NOTFND) return(YES);		// Also ignore if this api IN THIS DLL is explicitly specified
if (nolog_this_dll()) return(YES);			// OR THIS ENTIRE DLL is explicitly specified
return(NO);
}

static int logging(const char *fun)
{
if (!fun)	// the "shutdown" call
	{
	nolog_rls();
	dlllog=NOTFND;
	return(0);
	}

char w[256];
if (dlllog==NOTFND)
	{
		*w=0; //?//
	int opt=TOUPPER(*w);
	logopt=0;
	*logpath=0;
	if (opt=='R') logopt|=LOGOPT_ROOT;
	if (opt=='T') logopt|=LOGOPT_TIME;
	if (opt=='E') logopt|=LOGOPT_EXIT;
	dlllog = (logopt || opt=='Y');
	HDL f=flopen("C:\\NOLOG.TXT","R+");
	if (f)
		{
		while (!fleof(f))
			if (flgetln(w,sizeof(w),f)>0 && *_strupr(w))
				{
				if (!nolog) nolog=new DYNAG(0);
				nolog->put(w);
				}
		flclose(f);
		}
	if (dlllog && nolog) sjhloG("%s %s not logging API's specified in C:\\NOLOG.TXT",ctl2c(calnow()),caller_name);
	}
if (!dlllog || no_log(_strupr(strcpy(w,fun)))) return(NO);
return(YES);
}


#define LINEWIDTH 80
static void logparm1(const char *str)
{
char	w[256];
int		left=strlen(str), len, i;
do	{
	if ((len=strlen(str))>LINEWIDTH) len=LINEWIDTH;
	if (len) memmove(w,str,len);
	w[len]=0;
	for (i=0;w[i];i++)
		{
		int c=stridxc(w[i],"\t\r\n");
		if (c>=0) {w[i++]='\\';strinsc(&w[i],"trn"[c]);}
		else if (w[i]<' ') m_finish("Bad logging param - char=0x%02X",w[i]);
		}
	sjhLog("%s",w);
	str+=len;
	} while (*str);
}

static void LogParm(int io, DYNAG *arg)
{
if (logopt&LOGOPT_EXIT) return;
for (int i=0; i<arg->ct; i++)
	{
	LogArg *a=(LogArg*)arg->get(i);
	if (!(a->io&io)) continue;
	char *value, wrk[32];
	switch (a->type)
		{
		case 'I': strfmt(value=wrk,"%d",a->value); break;
		case 'C': if ((value=a->value)==NULL) strcpy(value=wrk,"(Null)"); break;
		case 'P': strfmt(value=wrk,"%d",*(int*)a->value); break;
		default: m_finish("Bad DllLog datatype");
		}
	sjhLog("P%d>",i);
	logparm1(value);
	sjhLog("%c</P%d>",'\f',i);
	}
}

static char *fmt_ftime(struct _timeb *t)
{
static char w[32];
char *ct=ctime(&t->time);
strfmt(w,"%8.8s",&ct[11]);
strendfmt(w,".%03d",t->millitm);
return(w);
}

char* DllLog::logtimetext(void)
{
static char txt[128];
static int then;
if (!(logopt&LOGOPT_TIME)) return("");

struct _timeb now;
_ftime(&now);
strfmt(txt," at %s",fmt_ftime(&now));
if (toggle)
	{
	int elap=((int)now.time-then)*1000 + (now.millitm-then_ms);
	strendfmt(txt,"  %d.%03d secs.",elap/1000, elap%1000);
	}
toggle=!toggle;
then=(int)now.time;
then_ms=now.millitm;
return(txt);
}

DllLog::DllLog(char *_func,int *_err,...)
{
toggle=0;
log_this_api=logging(_func);
if (!log_this_api) return;

if ((logopt&LOGOPT_ROOT) && !strcmp(_func,"DllInit"))
	{
	void __stdcall Xgetcwd(char *cwd);
	char pth[FNAMSIZ];
	Xgetcwd(pth);
	sjhLog("%s:cwd=%s",caller_name,pth);
	}

func=_func;
err=_err;
if (!(logopt&LOGOPT_EXIT)) sjhLog("%s:%s:IN%s",caller_name,func,logtimetext());
arg=new DYNAG(sizeof(LogArg));
va_list marker;
va_start(marker, _err);
char *v;
while ((v=va_arg(marker,char*))!=NULL)
	{
	int i;
	LogArg a={v[0],v[1]};
	i=TOUPPER(v[0]);
	if (stridxc(i,"ICP")==NOTFND) {sjhloG("Bad LogDataType(%c) from %s",v[0],func);m_finish("DllLog:A");}
	a.type=i;
	i=TOUPPER(v[1]);
	if ((i=stridxc(i,"IOB"))==NOTFND) {sjhloG("Bad LogScope(%c) from %s",v[1],func);m_finish("DllLog:B");}
	a.io=i+1;
	a.value=va_arg(marker,char*);
	arg->put(&a);
	}
LogParm(1,arg);
}

DllLog::~DllLog()
{
if (!log_this_api) return;
sjhLog("%s:%s:Ret=%d",caller_name,func,*err);
if (*err!=27)
	LogParm(2,arg);

if (*err)
	{
	char txt[512];
	DllGetLastError(txt);
	sjhLog("    ErrTxt:%s",txt);
	}
if (logopt&LOGOPT_TIME) sjhLog("%s",logtimetext());
delete arg;
}

