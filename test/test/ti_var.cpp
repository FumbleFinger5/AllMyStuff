#include "xlib.h"
#include "ti_var.h"
#include <io.h>


DYNAG *search_path;

TI_VAR *tiv;

const char *search_path0(void)
{
if (!search_path) return(".");		// ??? called before creating object ???
return((char*)search_path->get(0));
}

// TO DO: - this is slow!!! ##TryToFix##
static DYNAG *make_LastLoggedErrorTbl(void)
{
DYNAG	*t=new DYNAG(0);
char	str[FNAMSIZ];
HDL f=flopen(strfmt(str,"%s\\TI.log",search_path0()),"R");
if (f) 
	{
	while (flgetln(str,sizeof(str),f)>=0)
		{
//		if (*str=='\f') while (t->ct) t->del(t->ct-1);
		if (*str=='\f') {delete t;  t=new DYNAG(0);}	// ??? no better!
		else t->put(str);
		}
	flclose(f);
	}
return(t);
}

static void GetLastLoggedError(char *buffer)
{
static DYNAG *tbl=NULL;
if (buffer)
	{
	if (!tbl) tbl=make_LastLoggedErrorTbl();
	if (return_up_to_64K(buffer, tbl, fmt_str)) return;
	}
SCRAP(tbl);
}

int __stdcall DllGetLastLoggedError(char *buffer)
{
int err=NO;
DllLog z("DllGetLastLoggedError",&err,"co",buffer,0);
try
	{
//HDL tmr=tmrist(0);
	GetLastLoggedError(buffer);
//long csecs=tmrelapsed(tmr); tmrrls(tmr); if (*buffer) _sjhlog("%ld csecs",csecs); else _sjhlog("Done");
	}
catch (int errcode)
	{
	err=errcode;
	}
return(err);	// 0=No Error
}


HDL open_log_file(char *process_description)
{
char str[FNAMSIZ];
strfmt(str,"%s\\TI.log",search_path0());
int exists=drattrget(str,0);
HDL f=flopen_trap(str,"a");
if (exists) flputln("\f",f);
flputln(strfmt(str,"Process:%s  (run %s)",process_description,dmy_hm_str(calnow())),f);
return(f);
}

static void add_path(DYNAG *d, char *p)
{
char w[FNAMSIZ];
drfullpath(w,p);
FILEINFO fi;
if (drinfo(w,&fi) && (fi.attr&_A_SUBDIR)!=0)
	d->in_or_add(w);
}


static void fix_main_path(DYNAG *t, const char *param)
{
char	*p, *pth;
int	i,len, semicolon, pp;
for (i=0;i<t->ct;i++)
	if ((len=strlen(p=(char*)t->get(i)))>5 && SAME4BYTES(&p[len-4],"MAIN") && p[len-5]=='\\')
		return;

pth=_strupr(stradup(param));
for (p=pth; *p; p+=semicolon)
	{
	semicolon=stridxc(SEMICOLON,p);
	if (semicolon==NOTFND) semicolon=strlen(p);	// So next time we see EOS byte
	else p[semicolon++]=0;								// ELSE next time we see the character following the splatted semicolon
	if (*p)			// (ignore consecutive semicolons with no path in between...)
		for (i=pp=0;(i=stridxs("MAIN",&p[pp]))!=NOTFND;pp+=i)
			if (p[i+pp-1]=='\\' && p[i+pp+4]=='\\')
				{
				p[i+pp+4]=0;
				t->put(p,1);
				sjhLog("Warning - no MAIN path passed to %s:DllInit",caller_name);
				goto done;
				}
	}
done:
memtake(pth);
}


void set_search_path(const char *path)
{
if (!path)
	{SCRAP(search_path); SCRAP(tiv); return;}

char *pth;
pth=stradup(path);					// If it DID, make a copy so we can splat the semicolons to simplify parsing
search_path=new DYNAG(0);
int semicolon;
for (char *p=pth; *p; p+=semicolon)
	{
	semicolon=stridxc(SEMICOLON,p);
	if (semicolon==NOTFND) semicolon=strlen(p);	// So next time we look at EOS
	else p[semicolon++]=0;							// So next time we look at character following the splatted semicolon
	if (*p)			// (ignore consecutive semicolons with no path in between...)
		add_path(search_path,p);
	}
memtake(pth);
if (!search_path->ct) m_finish("Error setting Search path [%s]",path);
fix_main_path(search_path,path);
}


TI_VAR::TI_VAR(void)
{
if (!search_path) m_finish("Search path not set");
pth=new DYNAG(0);
for (int i=0;i<search_path->ct;i++)
	{
	char w[FNAMSIZ];
	drfulldir(w,(char*)search_path->get(i));
	strcat(w,"TI.INI");
	if (drattrget(w,0)) pth->put(w);
	}
}

TI_VAR::~TI_VAR()
{
delete pth;
}


#include <windows.h>
//	int GetShortPathName(char *LongPath, char *ShortPath_can_be_same_address, int short_path_max_chars_excluding_null);
//		returns length of shortPath_excluding_null
char *long2shortpath(char *str)
{
char s[FNAMSIZ];
int len=GetShortPathName(str, s, FNAMSIZ);
if (len>0 && len<(int)strlen(str))
	strcpy(str,s);
sjhLog("Obsolete Long2ShortPath invoked");
return(str);
}


static char *copy_value(char *s, char *w, int opt)
{
int i;
if (*w==34 && *(strend(w)-1)==34)
	{strdel(w,1); w[strlen(w)-1]=0;}
else
if ((i=stridxc(SEMICOLON,w))!=NOTFND) w[i]=0;
strcpy(s,strtrim(w));

if (opt&TIV_SHORTPATH) long2shortpath(s); // m_finish("Long2ShortPath not supported");

if ((opt&TIV_PATH) && (w=strend(s))!=s && (i=*(w-1))!=':' && i!='\\')
	*(short*)w='\\';		// add backslash to string if not already present
if (opt&(TIV_UPPER|TIV_PATH)) _strupr(s);
return(s);
}


// Copy TI.INI System Variable 'varnam' to 'value', returning 'value' as a convenience.
// Order of precedence is:-
//		1.) Use DOS environment variable if present,
// else 2.) Find the entry in TI.INI in CURRENT directory (if file exists),
// else 3.) Find the entry in TI.INI in MAIN directory (if file exists),
// else 4.) Find the entry in TI.INI in QUERIES directory (if file exists),
// else 5.) Set 's' to a null string
char* TI_VAR::get(const char *varnam, char *value, int opt)
{
char	*ev=getenv(varnam);
if (ev) return(copy_value(value,ev,opt));

int		len=strlen(varnam), multiline=0;
char	w[TEXT_LINE_SZ];
if (opt&TIV_MULTI) multiline=*value;

for (int sp=0; *value=0, sp<pth->ct; sp++)
	{
	int ok=0;
	char *fn=(char*)pth->get(sp);
	HDL f=flopen_trap(fn,"R");
	while (flgetln(w,TEXT_LINE_SZ-1,f)>=0)
		if (*w=='[') ok=(!_strnicmp(&w[1],varnam,len) && w[len+1]==']');
		else if (ok)
			{
			int i=0;
			do	copy_value(value,w,opt);
				while ((opt&TIV_MULTI)
					&&  (*(value=&strend(value)[1])=++i)<multiline
					&&  flgetln(w,TEXT_LINE_SZ-1,f)>=0 && *w!='[');
			ok=2;
			break;
			}
	flclose(f);
	if (ok==2) break;
	}
return(value);
}

// Locate file from a (possibly qualified) filename, looking in main & qry if necessary
char *findfile(char *fn)
{
static char fnam[FNAMSIZ];
if (stridxc('\\',fn)==NOTFND && stridxc(':',fn)==NOTFND)
	for (int sp=0; sp<search_path->ct; sp++)
		{
		strfmt(fnam,"%s\\%s",search_path->get(sp),fn);
		int found=drattrget(fnam,0);
		if (found)
			return(fnam);
		}
return(drfullpath(fnam,fn));
// TO_DO: Originally I thought fn should be appended to FIRST search_path entry if N/F
// I now think it's correct to expand without any path (if caller creates, it'll be in CWD)
}


void default_extn(char *p, const char *xtn)
{
int c,i,e;
i=e=strlen(p);
while ((c=p[--i])!='.')
	if (!i || c==':' || c=='\\') {p[e++]='.';MOVE4BYTES(&p[e],xtn);break;}
}

HDL flopen_search(char *fn)
{
return(flopen(findfile(fn),"R"));
}

