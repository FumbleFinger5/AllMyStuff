#ifdef docum
	Program:    WXP07					Ticket Archive Low-Level Access (Raw Data Load)
	System:     TRANSCEND
	Author:     Steve Harrison

Version
1.0.0	12/11/03	Created
#endif docum

#include "wxp07.h"

#include "dllr.h" //?//

char	*caller_name="WXP07";

//static	SqlDB *adb;

int __stdcall DllInit(char *path)
{
int err=NO;
DllLog z("DllInit",&err,"ci",path,0);
try
	{
	global_init(path);
	}
catch (int e)
	{
	err=e;
	}
return(err);				// 0 = No error
}

int __stdcall DllClosedown(void)
{
int err=NOTFND;
try
	{
	err=global_closedown();	// any unreleased memory block?
	}
catch (int e)
	{
	err=e;
	}
return(err);						// 0=No error, else maybe we've got leaks
}


#define MAX_TI_LENGTH 20
struct TI_SETTING
	{
	char	txt[MAX_TI_LENGTH+1];
	DYNAG	*pth;
	};



//?// #include <stdio.h>



static char *loadfile(char *fn)
{
FILEINFO fi;
char q[512];
void __stdcall Xgetcwd(char *cwd);
Xgetcwd(q);
if (drinfo(fn, &fi)!=YES)
	m_finish("Can't get %s attributes",fn);
int	sz=fi.size, got=0, i;
char *str=(char*)memgive(sz);
HDL f=flopen(fn,"R");
while (sz>0)
	{
	int sz1=MIN(sz,64000);
	flget(&str[got],sz1,f);
	sz-=sz1;
	while ((i=stridxs("\r\n",&str[got]))!=NOTFND) strdel(&str[got+i],2);
	while ((i=stridxs("\\t",&str[got]))!=NOTFND) {str[got+i]='\t';strdel(&str[got+i+1],1);}
	while ((i=stridxs("\\n",&str[got]))!=NOTFND) {str[got+i]='\n';strdel(&str[got+i+1],1);}
	sz1=strlen(&str[got]);
	got+=sz1;
	}
flclose(f);
return(str);
}
static void savefile(char *fn,char *str)
{
int	sz=strlen(str)+1;
HDL f=flopen(fn,"w+");
while (sz>0)
	{
	int sz1=MIN(sz,64000);
	flput(str,sz1,f);
	str+=sz1;
	sz-=sz1;
	}
flclose(f);
}

static int this_one(char *s)
{
char *x="the taking";
for (int i=0; s[i] && x[i];i++)
	if (s[i]!=x[i] && TOUPPER(s[i])!=TOUPPER(x[i]))
		return(NO);
x=x;
return(YES);
//return(!strncmp(s,x,strlen(x)));
}



static DYNAG *mn;		// Array of EMDB Movie names (year in brackets appended)

static int load_emdb(char *pth)
{
int i, ln;
char buf[500];
strfmt(buf,"%s%s",pth,"\\emdb.txt");
HDL f=flopen(buf,"R");
if (!f) return(NOTFND);
mn=new DYNAG(0);
for (i=0;i<2;i++)
	{
	ln=flgetln(buf,500,f);
	if (!i && !SAME4BYTES(buf,"Year")) return(NOTFND); // Not the right file, or UNICODE when expecting UTF-8
	}
while (flgetln(buf,500,f)>5)
	{
	int yr=a2i(buf,4);
	if (buf[4]!=SPACE) return(NOTFND);
	strtrim(strdel(buf,4));
	strendfmt(buf," (%04.4d)",yr);
//if (this_one(buf)) yr=yr;
	mn->put(buf);
	}
if (!mn->ct) return(NOTFND);
flclose(f);
return(0);
}

static int in_emdb(char *fn)
{
char buf[500];
strcpy(buf,fn);
int i=0,
	ln=strlen(buf),
	yr=a2i(&buf[ln-5],4);

if (ln<8 || buf[ln-1]!=')' || buf[ln-6]!='(' || buf[ln-7]!=SPACE || yr<1910 || yr>2030)
	{Xecho("Bad year in %s\r\n",fn);return(NOTFND);}

if (this_one(fn))
i=i;

char *p;
for (i=0;(p=(char*)(mn->get(i)))!=NULL;i++)
	{
	if (!stricmp(p,buf))
		return(i);
	}
i=i;
return(NOTFND);
}


struct CMFC {short fct, dct, dvd; __int64 sz;};

/*static void fc(char *a, char *b)
{
	int i,c1,c2;
	for (i=0;a[i] || b[i];i++)
	{
		c1=a[i];
		c2=b[i];
		if (c1!=c2)
			i=i;
	}

}*/

static int check_movie_folder_contents(CMFC *cmfc, char *path)
{
char pth[256], *pe;
pe=strend(strcpy(pth,path));
strcpy(pe,"\\*.*");
DRSCN *scn=drscnist(pth);
FILEINFO fi;
while (drscnnxt(scn,&fi))	// Scan each folder in this "Film" subfolder
	{
	if (fi.attr&FA_DIR)
		{
		if (!strcmp(strcpy(pe,fi.name),"VIDEO_TS")) {cmfc->dvd=YES;break;}
		cmfc->dct++;
		check_movie_folder_contents(cmfc,pth);
		}
	else
		{
		cmfc->fct++;
		cmfc->sz += fi.size;
		}
	}
drscnrls(scn);
return(0); // no error
}

static int disc_num;
static char disc_nam[20];

static int find_first_backslash(char *pth)
{
int i=0,twice=NO;
if (SAME2BYTES(pth,"\\\\")) twice=i=2;
while (pth[i]) if (pth[i++]=='\\') break;
if (twice) while (pth[i]) if (pth[i++]=='\\') break;
i=i;
return(i);
}

static int verify_root(char *path)
{
FILEINFO fi;
if (!drinfo(path,&fi) || (fi.attr&FA_DIR)==0)
	{
	Xecho("%s is't a directory\r\n",fi.name);
	return(NOTFND);
	}
char pth[FNAMSIZ];
int i=find_first_backslash(strcpy(pth,path));
//for (i=strlen(strcpy(pth,path)); i>0 && pth[i-1]!='\\'; i--) {;}
strcpy(&pth[i],"Label*.txt");
if (drinfo(pth,&fi) && fi.name[5]==SPACE && fi.name[8]==SPACE && (i=strlen(fi.name))<25)
	{
	disc_num=a2i(&fi.name[6],2);
	if (fi.name[i-4]=='.')
		{
		fi.name[i-4]=0;
		strcpy(disc_nam,&fi.name[9]);
		return(0); // No Error
		}
	}
Xecho("Label NN xxxxxxx.txt not found in root of %s\r\n",path);
return(1); // bad label file
}

//#define CHECK_DATES
static int scan_movies(char *path)
{
int ct=0,er;
FILEINFO fi;
DRSCN *scn;
if ((er=verify_root(path))!=0) return(er);
char pth[FNAMSIZ],*pe;
pe=strend(strcpy(pth,path));
strcpy(pe,"\\*.*");
scn=drscnist(pth);
while (drscnnxt(scn,&fi))	// Scan each folder in this "Films" folder
	{
	if (*fi.name=='_') continue;	// Ignore any files/folders starting with underscore
	if (fi.attr&FA_DIR)
		{
		int fnd=(in_emdb(fi.name)!=NOTFND);
		if (!fnd) {ct++; Xsjhlog(fi.name);}
#ifndef CHECK_DATES
		CMFC cmfc={0,0,0,0};
		strfmt(pe,"\\%s",fi.name);
		check_movie_folder_contents(&cmfc, pth);
		long sz=cmfc.sz/(1024*1024);
		if (!cmfc.dvd)
		if (sz<1 || cmfc.dct>0)
			Xecho("%s  sz:%ld %s\r\n", fi.name,sz,(cmfc.dct?"contains subfolders":""));
#endif // CHECK_DATES (code not included when making initial check)
		}
	else Xecho("%s NOT dir\r\n",fi.name);
	}
drscnrls(scn);
Xsjhlog(strfmt(pth,"%d movies not in EMDB listed\r\n",ct));
return(0);
}

static char epth[200];

static int set_paths(char *pth)
{
//strcpy(pth,"\\\\egg\\f\\Video\\Films");
strcpy(pth,"\\\\egg\\e\\newFilms");
//strcpy(pth,"G:\\Video\\Films2");
int ct=0;
char *user=0;
FILEINFO fi;
DRSCN *scn=drscnist(strcpy(epth,"C:\\Users\\*.*"));
while (drscnnxt(scn,&fi))
	if ((fi.attr&FA_DIR))
		if (!stricmp(fi.name,"Steve")) {user="Steve";break;}
		else if (!stricmp(fi.name,"Harri")) {user="Harri";break;}
drscnrls(scn);
if (!user) return(NOTFND);
strfmt(&epth[8],"\\%s\\Dropbox\\shared",user);
return(0);
}

int main(int argc, char* argv[])
{
char	vbuf[VBUFSZ], dttm[32], fnam[128];
int		i, er=0, bombed=0, echoed=0, cfg;

int		opt=1;

char	w[1024], pth[FNAMSIZ], cfgstr[32];
int		modfil;
*w=0;	// (just so it's not 'unreferenced')

if ((er=set_paths(pth))!=0) goto fail;
/*
if (argc>1)
	strcpy(pth,argv[1]);
if (argc>2)
	opt=a2i(argv[2],2);
*/

if ((er=DllInit(epth))!=0) goto fail;
if ((er=DllRegisterStatusFn(XStatusCallback))!=0) goto fail;
if ((er=DllRegisterCancelFn(XCancelCallback))!=0) goto fail;


if (opt)
	{
	switch (opt)
		{
		case 1:
//			if ((er=load_emdb("D:\\V1\\emdb.txt"))!=0) break;
			if ((er=load_emdb(epth))!=0) break;
			er=scan_movies(pth);
			break;
		case 2:
			break;
		default: er=996;
		}
	if (er) goto fail; else goto skip;
	}

	
if (er) goto fail;


goto skip;

fail:
char e[1000];
DllGetLastError(e);
Xecho("Error:%d  %s\n",er,e);
bombed=1;

skip:
if (!echoed++) Xecho(0);
if (!bombed)
	if ((er=DllClosedown())!=0)
		{
		Xecho("Memory leaks detected!\n");
		goto fail;
		}
Xwait("Press any key to exit\n");
return(er);
}
