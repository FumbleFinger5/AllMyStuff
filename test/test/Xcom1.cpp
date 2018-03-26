#include "XLIB.H"
#include "dll.h"
#include "dgrp.h"
#include "ti_var.H"


// PTR to Status callback function implemented in the GUI.
// "total"	= size of task (total number of discs to process)
// "done"	= progress (number of discs processed so far)
static void  (__stdcall *ptr_StatusCallback)(int total, int done);

// Save pointer to function implemented in the GUI. Returns 0 on success
int __stdcall DllRegisterStatusFn(void (__stdcall *funcptr)(int, int))
{
ptr_StatusCallback = funcptr;
return(NO);						// No errors possible here...
}

void	report_status(int done_so_far, int total)
{
if (ptr_StatusCallback)
	{
	(ptr_StatusCallback)(done_so_far,total);
	}
}

// PTR to cancel callback function implemented in the GUI.
// Returns non-zero if cancel is requested
static int (__stdcall *ptr_CancelCallback)(void);

// Save pointer to function implemented in the GUI. Returns 0 on success
int __stdcall DllRegisterCancelFn(int (__stdcall *funcptr)(void))
{
ptr_CancelCallback = funcptr;
return(NO);						// No errors possible here...
}

void	GetTitle(char *vbuf, char *title)
{
int		p;
strcpy(vbuf,title);
while ((p=stridxc('\r',vbuf))!=NOTFND)
	{
	vbuf[p++]=TAB;
	while (vbuf[p]==LNFEED) strdel(&vbuf[p],1);
	}
}


PROGRESS::PROGRESS(int pre_subtask, int post_subtask)
{
pre=force_into_range(pre_subtask,0,100);
post=force_into_range(post_subtask,0,100-pre);
subtask_max=100-(pre+post);
total=0;
last_percent=NOTFND;
}

void PROGRESS::report(int done, int tot)
{
if (tot>0) total=tot;
if (total<0) total=0;
done=force_into_range(done, 0,total);
int done_percent=pre;
if (subtask_max && done)
	done_percent+=((done*subtask_max)/total);
if (done_percent!=last_percent)
	report_status(last_percent=done_percent,100);
}


int	user_cancel(int dont_crash)
{
int cancel=NO;
if (ptr_CancelCallback)
	{
	cancel=(ptr_CancelCallback)();
	if (cancel)
		{
		if (dont_crash) return(YES);
		throw RetErrorText(27,"Operation cancelled by User");
		}
	}
return(NO);
}

int stripln(char *s, HDL f)
{
int i;
flgetln(s,128,f);
if ((i=stridxc(SEMICOLON,s))!=NOTFND) s[i]=0;
return(*strtrim(s));
}

int rollover_adjust(int hm)
{
if (hm<rollover) hm+=1440;
return(hm);
}

int a2hm2(int h, int m)
{
if (h<24 && m<60)
	return(rollover_adjust(h*60+m));
return(NOTFND);
}


int a2hm(const char *p)	// fast & loose binary minutes from hhmm string
{
short h = a2i(p,2);
if (!a2err)
	{
	short m = a2i(&p[2],2);
	if (!a2err)
		return(a2hm2(h,m));
//		return(a2hm2(h,m)); // %1440);
	}
return(NOTFND);
}

int a2hmv(const char *hhmm)	// Verify time is a valid hhmm + NULL
{							// return NOTFND if error
int m=a2hm(hhmm);
return((SAME4BYTES(hm_stri(m),hhmm) && !hhmm[4])?m:NOTFND);
}

int x2hm(const char *p)
{
return(rollover_adjust(((p[0]>>4)*600+(p[0]&15)*60+(p[1]>>4)*10+(p[1]&15))%1440));
}

int x2hm0(const char *p)
{
return(x2hm(p)%1440);
}


int x2hmv(const char *p)
{
int h=x2l(p,0,2);
if (a2err) return(NOTFND);
int m=x2l(&p[1],0,2);
if (a2err) return(NOTFND);
h=a2hm2(h,m);
if (h!=NOTFND) h%=1440;
return(h);
}



int day_bin(char *day)	// Return daysop bitswitch from 7 x Y/N flags
{						// First flag is ALWAYS Sunday
int   i,bitflag;
for (i=bitflag=0;i<7;i++)
	if (TOUPPER(day[i])=='Y')
		bitflag|=(1<<i);
return(bitflag);
}

int get_1_dep(const char *mask)
{
int dep=NOTFND;
for (int i=0;i<60;i++)
	if (bit_test(mask,i))
		if (dep==NOTFND) dep=i;
		else m_finish("depot bitflag error!");
if (dep==NOTFND) m_finish("depot bitflag error!!");
return(dep);
}

void c2mask(char *mask, const char *str)	// Set 8-byte (64-bitflag) 'dep' from 60 x Y/N flags
{
BIT8_SET0(mask);
for (int i=0;i<60;i++) if (TOUPPER(str[i])=='Y') bit_set(mask,i);
}

char *mask2c(const char *mask)	// Set 60 x Y/N flags from 8-byte (64-bitflag) 'dep' mask
{
static char str[61];
for (int i=str[60]=0; i<60; i++)
	str[i]=(bit_test(mask,i)?'Y':'N');
return(str);
}

int _cdecl cp_mask(const char *a, const char *b)
{
int i=60,cmp;
while (i--)
	if ((cmp=bit_test(a,i)-bit_test(b,i))!=0) break;
return(cmp);
}

int _cdecl cp_date_dep_scope(DATE_DEP_SCOPE *a, DATE_DEP_SCOPE *b)
{
int cmp=cp_ushort_v(a->intro,b->intro);
if (!cmp) cmp=cp_mask(a->mask,b->mask);
return(cmp);
}



char  *day_str(int day)		// Return 7 Y/N flags from int flag
{							// First flag is ALWAYS Sunday
static   char ch[8];
int   i;
for (i=ch[7]=0;i<7;i++) ch[i]=(day & (1<<i))?'Y':'N';
return(ch);
}

int dopb2dop06(int dop)		// Convert input bitflag with ONE day-bit turned on to a subscript 0-6
{							// From 1=Sun, 2=Mon, 4=Tue,...
for (int i=7;i--;)			// To   0=Sun, 1=Mon, 2=Tue,...
	if (dop==(1<<i))
		return(i);
sjhLog("Internal warning - invalid DaysOp flag:%d",dop);
//m_finish("Internal error - invalid DaysOp flag");
return(0);
}

int daynam2num(char *daynam)
{
int i,len=strlen(daynam);
for (i=7;i--;)
	if (len>1 && !_strnicmp(daynam,sy_daynam[i],len))
		break;
if (i==NOTFND)
	{
	sjhLog("Bad day name [%s]",daynam);
	m_finish("Bad day name [%s]",daynam);
	}
return(1<<i);
}

#define DAYNAME sy_daynam[d%7]	// yawn 13/10/03
#define ALL_DAYS 127

// 'dop' must NOT be passed as a Dop_adj - adjusted value!
// this s/r expects dop bits 1,2,4,..64 to be Sun,Mon,Tue,...Sat
void build_daysop(int dop, char *w)
{
int d,gap;
*w=0;
if (dop==ALL_DAYS) return;
for (d=7;d--;) if ((dop|(1<<d))==ALL_DAYS) {strfmt(w,"Not %3.3s",DAYNAME);return;}
for (d=0;d<7;d++) if (dop&(1<<d)) break;
for (gap=0;d<7;d++)
	if ((dop&(1<<d))==0) gap=1;
	else if (gap) {for (d=*w=0;d<7;d++) if (dop&(1<<d)) strancpy(strend(w),DAYNAME,3);}
	else {if (*w) strcpy(&w[3],"-"); strancpy(strend(w),DAYNAME,4);}
}
#undef DAYNAME
#undef ALL_DAYS


// Return string representation of passed DOPTERM in local static string
char *dopterm2str(DOPTERM *dt)
{
static char str[32];
if (dt->term==3) str[0]=0; else strcpy(str,(dt->term==1)?"Term":"Holiday");
if (dt->dop&128)
	strendfmt(str,get_dop_special(dt->dop&127));
else
	build_daysop(dt->dop, strend(str));
return(str);
}



void item_del(void *a, int ct, int i, int sz)
{
if (i<ct)
	memmove(&((char*)a)[i*sz],&((char*)a)[(i+1)*sz],(ct-i)*sz);
}

int ti_var_y_once(int *flag, char *ti_name)
{
if (*flag==NOTFND)
	*flag=ti_var_y(ti_name);
return(*flag);
}

extern	char	*curv;
extern	int		log200;
char *buff;
char *bp;

#include "dllr.h"

void	setup_sy(int init);
void	use_StdSY(void);
void	sy_init(void);
void	__stdcall Xchdir(char *pth);

void global_init(const char *main_pth)
{
leak_tracker(YES);
//char cwd[FNAMSIZ];
//Xgetcwd(cwd);

//?// ::CoInitialize(NULL);			// Initialise OLE.
//set_search_path("d:\\v1");
set_search_path(main_pth);
tiv=new TI_VAR();

buff=(char*)memgive(65000*4);
dbstart(32); //32);

extern int do_memchk;
if (ti_var_y("memchk"))
	do_memchk=YES;
log200=ti_var_y("LOG200");
char wrk[128];
if (*ti_var("DBSAFE",wrk,TIV_UPPER)=='N') dbsafe_catch=0; else
dbsafe_catch=DB_IS_DIRTY;
setup_sy(YES);
sy_init();
}

int global_closedown(void)
{
if (log200) sjhLog("Warning! - redundant [Log200]=Y setting in TI.INI");
DllSuppress("");
setup_sy(NO);
dbstop();
clear_callbacks();
XStatusCallback(NOTFND,NOTFND);
Scrap(curv);
Scrap(buff);
set_search_path(0);
int err=NO;
Xecho(0);
err=leak_tracker(NO);

extern int	dlllog;
if ((dlllog>0 && !nolog_this_dll()) || err)
	{
	delete new DllLog(0,0);
	extern int first_leak;
	char str[256];
	strcpy(str,"global_closedown ");
	if (err) strendfmt(str,"%d memory leaks! First=%d",err,first_leak);
	else strcat(str,"OK");
	sjhLog("%s:%s",caller_name,str);
	}

sjhLog(0);	// clears 'current log file' path
//?// ::CoUninitialize();
return(err);
}


int hex2buff(char *hex)
{
int		i,nibble, ct;
char	ch,c;
for (i=nibble=ct=ch=0;(c=hex[i])!=0;i++)
	{
	if (c>='0' && c<='9') c-='0';
	else if ((c=TOUPPER(c))>='A' && c<='F') c-=('A'-10);
	else continue;
	if (nibble++) {buff[ct++]=(ch<<4) | c; nibble=0;}
	ch=c;
	}
if (nibble) m_finish("Odd number of hex-encoded bytes!");
return(ct);
}

int contract_end(const char *contract)
{
return(!*contract || SAME2BYTES(contract,"0"));
}


