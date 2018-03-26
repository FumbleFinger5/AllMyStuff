#ifdef docum
	Program:    WXP07					Ticket Archive Low-Level Access (Raw Data Load)
	System:     TRANSCEND
	Author:     Steve Harrison

Version
1.0.0	12/11/03	Created
#endif docum

#include "wxp07.h"

#include "dllr.h" //?// (call thru X- wrappers, not directly into pf_lib)

char	*caller_name="WXP07";

char epth[200];

int sinker(char *pth1, char *pth2);


static int disc_num;
static char disc_nam[20], disc_pth[80];

//struct CMFC {short fct, dct, dvd; long sz; char *mn;};
static DYNAG *em;		// Array of EMDB Movie names (year in brackets appended)

int update_dbf(int disc_num, char *disc_pth, DYNAG *em);



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


class PARSER {
public:
//PARSER();						// Constructor used by app progs
//~PARSER() {;};
int	first_line(char *buf);
int	make_full_moviename(char *buf);

private:
int		title_fldno, year_fldno;
};

int PARSER::first_line(char *buf)
{
int i,fldno;
title_fldno=NOTFND; year_fldno=NOTFND;
for (i=fldno=0;buf[i];i++)
	{
	if (buf[i]==':') fldno++;
	if (!memcmp(&buf[i],"Title",5)) title_fldno=fldno;
	if (!memcmp(&buf[i],"Year",4)) year_fldno=fldno;
	}
if (title_fldno==NOTFND || year_fldno==NOTFND)
	return(NOTFND);
return(0); // no error
}

int PARSER::make_full_moviename(char *buf)
{
char w[100];
int i,j,sep;
for (i=sep=0;buf[i] && sep<year_fldno;i++) if (buf[i]==':') sep++;
i=i;
int yr=a2i(&buf[i],4);
for (i=sep=0;buf[i] && sep<title_fldno;i++) if (buf[i]==':') sep++;
for (j=0;(w[j]=buf[i])!=0 && buf[i++]!=':'; j++) {;}
w[j]=0;
strtrim(strcpy(buf,w));
strendfmt(buf," (%04.4d)",yr);
return(0); // no error
}


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
char *x="A Million";
for (int i=0; s[i] && x[i];i++)
	if (s[i]!=x[i] && TOUPPER(s[i])!=TOUPPER(x[i]))
		return(NO);
x=x;
return(YES);
//return(!strncmp(s,x,strlen(x)));
}

static int load_emdb(char *pth)
{
int i, ln;
char buf[500];
strfmt(buf,"%s%s",pth,"\\emdb.csv");
HDL f=flopen(buf,"R");
if (!f) return(NOTFND);
em=new DYNAG(sizeof(CMFC));
PARSER p;
flgetln(buf,500,f);	// skip the initial "fieldnames" line in *.csv
if (p.first_line(buf))
	{
	Xecho("Error reading %s EMDB.CSV (maybe not UNICODE - open in Notepad & re-save)\n",pth);
	return(NOTFND); // Not the right file, or UNICODE when expecting UTF-8
	}
flgetln(buf,500,f); // skip the second line (it's just underscores)

while (flgetln(buf,500,f)>5)
	{
	if (p.make_full_moviename(buf))
		{Xecho("Bad name %s\r\n",buf);return(NOTFND);}
	CMFC c;
	memset(&c,0,sizeof(CMFC));
	c.mn=(char*)memgive(strlen(buf)+1);	// NOT strdup, 'cos it uses a different memory allocator!
	strcpy(c.mn,buf);
	em->put(&c);
	}
if (!em->ct) return(NOTFND);
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
CMFC *p;
for (i=0;(p=(CMFC*)(em->get(i)))!=NULL;i++)
	{
if (this_one(p->mn))
i=i;

	if (!stricmp(p->mn,buf))
	return(i);
	}
return(NOTFND);
}


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

ulong filesize(char *s)
{
GET_FILEEX_INFO_LEVELS fInfoLevelId = GetFileExInfoStandard;
WIN32_FILE_ATTRIBUTE_DATA f;
int ret;
//ret = GetFileAttributesEx((LPCWSTR)File.c_str(), fInfoLevelId, &lpFileInformation);
//ret = GetFileAttributesEx((LPCSTR)s, fInfoLevelId, &lpFileInformation);
ret = GetFileAttributesEx((LPCSTR)s, GetFileExInfoStandard, &f);
// __int64
__int64 *p=(__int64*)&f.nFileSizeHigh;
ulong sz=(*p) / (1024 * 1024);
ulong lo=f.nFileSizeLow,hi=f.nFileSizeHigh;
ulong x=lo / (1024 * 1024);
ulong xx=lo>>20;
ulong hh=hi<<12;
ulong zz=xx|hh;
return(zz);
}

static int check_movie_folder_contents(CMFC *cmfc, char *path)
{
char pth[256], *pe;
pe=strend(strcpy(pth,path));
strcpy(pe++,"\\*.*");				// Now points AFTER the backslash!!!
DRSCN *scn=drscnist(pth);
FILEINFO fi;

if (this_one(cmfc->mn))
	pe=pe;

while (drscnnxt(scn,&fi))	// Scan each folder in this "Film" subfolder
	{
	strcpy(pe,fi.name);
	if (fi.attr&FA_DIR)
		{
		if (!strcmp(fi.name,"VIDEO_TS")) {cmfc->dvd=YES; /*break;*/ }
		cmfc->dct++;
		check_movie_folder_contents(cmfc,pth);	// recursive call (find actual video files in the subfolder)
		}
	else
		{
		cmfc->fct++;
		cmfc->sz += filesize(pth);
		}
	}
drscnrls(scn);
return(0); // no error
}


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
	Xecho("%s is't a directory\r\n",path);
	return(NOTFND);
	}
char pth[FNAMSIZ];
int i=find_first_backslash(strcpy(pth,path));
//for (i=strlen(strcpy(pth,path)); i>0 && pth[i-1]!='\\'; i--) {;}
strcpy(disc_pth,&path[i]);
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

if (this_one(fi.name))
	pe=pe;

	if (fi.attr&FA_DIR)
		{
		int fnd=in_emdb(fi.name);
		if (fnd==NOTFND)
			{ct++; sjhlog("Movie not in EMDB:%s\r\n",fi.name); continue;}
#ifndef CHECK_DATES
		CMFC *cmfc=(CMFC*)em->get(fnd);
		strfmt(pe,"\\%s",fi.name);
		check_movie_folder_contents(cmfc, pth);
		if (cmfc->sz<1 || (cmfc->dct>0 && !cmfc->dvd))
			Xecho("%s  sz:%ld %s\r\n", fi.name,cmfc->sz,(cmfc->dct?"contains subfolders":""));
#endif // CHECK_DATES (code not included when making initial check)
		}
	else Xecho("%s NOT dir\r\n",fi.name);
	}
drscnrls(scn);
sjhlog("%d movies not in EMDB listed\r\n",ct);
/*
int i,ii,j,sz;
for (i=sz=0;i<mn->ct;i++)
	if ((j=strlen((char*)mn->get(i)))>sz) {sz=j; ii=i;}
Xecho("%d chars [%s]\r\n",sz,(char*)mn->get(ii));
*/

er=update_dbf(disc_num, disc_pth, em);

int nct=0, dvd_ct=0, tsz=0;
for (int i=0;i<em->ct;i++)
	{
	CMFC *c=(CMFC*)em->get(i);
//struct CMFC {short fct, dct, dvd; long sz; char *mn;};
	if (0)
if (c->dvd || c->sz)
sjhlog("%s fct:%d dct:%d dv:%d sz:%d",c->mn,c->fct,c->dct,c->dvd,c->sz);
	if (c->dvd) {dvd_ct++; nct++;}
	else if (c->sz) {tsz+=c->sz; nct++;}
	memtake(c->mn);
	}
SCRAP(em);
if (!ct)
	Xecho("%d movies (incl. %d DVDs) Total %dMb\r\n",nct,dvd_ct,tsz);
return(er);
}

// 2,147,483,647 = maxint  int64
#ifdef FAILS
static __int64	 get_folder_size(char *path, char *subpath)
{
char p[200];
strfmt(p,"%s\\%s\\*.*",path,subpath);
__int64	 sz=0;
DRSCN *scn=drscnist(p);
FILEINFO fi;
while (drscnnxt(scn,&fi))
	{
	if ((fi.attr&FA_DIR)==0)
		sz+=fi.size;
	else
		sz+=get_folder_size(p,fi.name);
	}
drscnrls(scn);
return(sz);
}
#else

__int64 fsll(char *s)
{
GET_FILEEX_INFO_LEVELS fInfoLevelId = GetFileExInfoStandard;
WIN32_FILE_ATTRIBUTE_DATA f;
int ret;
//ret = GetFileAttributesEx((LPCWSTR)File.c_str(), fInfoLevelId, &lpFileInformation);
//ret = GetFileAttributesEx((LPCSTR)s, fInfoLevelId, &lpFileInformation);
ret = GetFileAttributesEx((LPCSTR)s, GetFileExInfoStandard, &f);
__int64 sz, lo=f.nFileSizeLow, hi=f.nFileSizeHigh;
sz=((hi<<32) | lo);
return(sz);
}


static __int64	 get_folder_size(char *path, char *subpath)
{
char p[200];
strfmt(p,"%s\\%s",path,subpath);
char *pe=strend(p);
strcpy(pe,"\\*.*");
__int64	 sz=0;
DRSCN *scn=drscnist(p);
FILEINFO fi;
while (drscnnxt(scn,&fi))
	{
	if ((fi.attr&FA_DIR)==0)
		{
		strfmt(pe,"\\%s",fi.name);
		sz+=fsll(p);
		}
	else
		{
		*pe=0;
		sz+=get_folder_size(p,fi.name);
		}
	}
drscnrls(scn);
return(sz);
}
#endif


static char *longlong2str(__int64 n)	// TOTAL KLUDGE! - but it should work!
{
static int sw;
static char s4[4][32];
char *s=s4[sw++ & 3];
if (n==0) strcpy(s,"0");
else
	{
	*s=0;
	while (n!=0)
		{
		int digit=n%10;
		n/=10;
		strinsc(s,digit+48);
		}
	}
return(s);
}

#include <direct.h>
#include <io.h>

static DRSCN *drscnist_all(char *pth)
{
char p[200];
strfmt(p,"%s\\*.*",pth);
return(drscnist(p));
}

static int delete_folder(char *fullpath)
{
int err=NO;
DRSCN *scn = drscnist_all(fullpath);
FILEINFO fi;
while (!err && drscnnxt(scn,&fi))	// Scan each entry in this folder
	{
	char pp[200];
	strfmt(pp,"%s\\%s",fullpath,fi.name);
	if (fi.attr&FA_DIR)		// this is a directory - recursively call to empty the directory before deleting
		{
		err=delete_folder(pp);
		}
	else
		{
		err=_unlink(pp);
		}
	}
drscnrls(scn);
if (!err)
	err=_rmdir(fullpath);
return(err);
}

// Currently doesn't preserve dttm of source file!!!!!!!!!!!!
// ########### ALSO WROTE BLAZING SADDLES.SRT AS UNICODE!!!!!!!!!!!!!!!!!!!!!!!!!!!
#ifdef UNUSED
static int copy_file(char *src, char *fname, char *dst)	// NOT NEEDED
{
int sz1,sz2;
char pp[200];
strfmt(pp,"%s\\%s",src,fname);

// __int64 src_sz=fsll(pp);

HDL f_src=flopen(pp,"R");
if (f_src==0) return(-2);
HDL f_dst=flopen(strfmt(pp,"%s\\%s",dst,fname),"w");
if (f_dst==0) return(-3);
char bf[4096];
ushort sz;
while (!fleof(f_src) && (sz=flget(bf,4096,f_src))>0)
	flput(bf,sz,f_dst);
flclose(f_dst);
flclose(f_src);

// __int64 dst_sz=fsll(pp); if (src_sz!=dst_sz) m_finish("SIZEDIFF! %s",pp);

return(0);
}
#endif // UNUSED

static int copy_file(char *_src, char *fname, char *_dst)
{
char src[200], dst[200];
strfmt(src,"%s\\%s",_src,fname);
strfmt(dst,"%s\\%s",_dst,fname);
int success=CopyFile(src,dst,YES);	// Bool param3 = "fail if destination file already exists"
return(!success);					// From MS documentation: "If the function (CopyFile) succeeds, the return value is nonzero"
}									// - so reverse the "success" flag setting and return a more useful ERROR value

static int copy_folder1(char *_src, char *fname, char *_dst)
{
int err=NO;
char src[200], dst[200];
strfmt(src,"%s\\%s",_src,fname);
strfmt(dst,"%s\\%s",_dst,fname);
DRSCN *scn = drscnist_all(src);
FILEINFO fi;
err=mkdir(dst);
while (!err && drscnnxt(scn,&fi))	// Scan each entry in this folder
	{
	if (fi.attr&FA_DIR)		// this is a directory - recursively call to copy the files/folders therein
		{
		err=copy_folder1(src, fi.name, dst);
		}
	else
		{
		Xecho("..");
		err=copy_file(src,fi.name,dst);
		}
	}
drscnrls(scn);
return(err);
}

static int copy_folder(char *_src, char *fname, char *_dst)
{
short attr;
int err=NO;
char src[200], dst[200];
strfmt(src,"%s\\%s",_src,fname);
strfmt(dst,"%s\\%s",_dst,fname);
Xecho("COPY %s to %s ...",src,_dst);
if (drattrget(dst,&attr))
	if (attr & FA_DIR)
		err=delete_folder(dst);
	else m_finish("%s is a FILE (expected FOLDER)", dst);
if (!err)
	{
	err=copy_folder1(_src, fname, _dst);
	}
Xecho("DONE!\n");
return(err);
}


// pth2=the "wanted" folder, pth1 MAY contain new/updated movie folders to be copied into pth2
void test4(char *src_pth, char *dst_pth) // Match and list all folders in pth1, specifying LT, EQ, GT, NF (notfnd) relative to the same (individual movie) folder in pth2
{
__int64 sz1,sz2;
int err=0, ct[4]={0,0,0,0};
char p[200];
int status;
char *statx[4] = {"NO","GT","LT","EQ"};
strfmt(p,"%s\\*.*",src_pth);
DRSCN *scn=drscnist(p);
FILEINFO fi;
while (drscnnxt(scn,&fi))
	{
	if ((fi.attr&FA_DIR)==0) Xecho("Ignoring source FILE %s %s\n",src_pth,fi.name); // not interested in files, only folders
	else
		{
//if (SAME4BYTES(fi.name,"McCa")) sz1=sz1;
		sz1=get_folder_size(src_pth,fi.name);
		sz2=get_folder_size(dst_pth,fi.name);
		if (sz2==0) status=0; else if (sz1>sz2) status=1; else if (sz1<sz2) status=2; else status=3;
		} // status = 0/1 = destination missing / smaller (DO COPY). status = 2/3 = destination smaller / equal size (DON'T COPY)
	ct[status]++;
	sjhlog("Src %s Dst: %s %s %s/%s\r\n",statx[status],dst_pth,fi.name,longlong2str(sz1),longlong2str(sz2));
//	sjhlog("COPY2 %s\\%s %s",dst_pth,fi.name,pth1);
	if (status<=1)
		err=copy_folder(src_pth, fi.name, dst_pth);
	if (err) m_finish("FAILED COPY %s\\%s %s",src_pth,fi.name,dst_pth);
	}
drscnrls(scn);
Xecho("Would copy %d folders (%d new, %d bigger)\n",ct[0]+ct[1],ct[0],ct[1]);
Xecho("Ignoring %d folders (%d smaller, %d same size)\n",ct[2]+ct[3],ct[2],ct[3]);
}


static int set_emdb_path(void)
{
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



static void test_unicode(void)
{
int i=0;
copy_file("D:\\", "test.srt", "D:\\X");
i++;
}


int main(int argc, char* argv[])
{
char	vbuf[VBUFSZ], dttm[32], fnam[128];
int		i, er=0, bombed=0, echoed=0, cfg;

int		opt=2;

char	w[1024], pth[FNAMSIZ], cfgstr[32];
int		modfil;
*w=0;	// (just so it's not 'unreferenced')

//er=sinker("G:\\newfilms","E:\\newfilms"); // ??? no idea what this was for!!!
//goto skip;

if ((er=set_emdb_path())!=0) goto fail;

//strcpy(pth,"\\\\egg\\f\\Video\\Films");
strcpy(pth,"G:\\video\\films");

if ((er=DllInit(epth))!=0) goto fail;
if ((er=DllRegisterStatusFn(XStatusCallback))!=0) goto fail;
if ((er=DllRegisterCancelFn(XCancelCallback))!=0) goto fail;


if (opt)
	{
	switch (opt)
		{
		case 1:
			if ((er=load_emdb(epth))!=0) break;
			if ((er=scan_movies(pth))!=0) break;
// export to database the contents of em[] (composite key: disc_num+disc_pth + MovieName+Year
			break;
		case 2:
			{
//			SMDB s(strfmt(vbuf,"%s\\%s",epth,"SMDB.dbf"));
			test4(pth,"E:\\newfilms");
			}
			break;
		case 3:	// check all folders in pth1
			if ((er=load_emdb(epth))!=0) break;
			if ((er=scan_movies(pth))!=0) break;
			break;
		case 4:
			test_unicode();
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
