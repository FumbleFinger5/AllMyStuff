#include "pdefs.h"
#include <windows.h>
#include <io.h>
#define MSDOS 1
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>
#include <direct.h>
#include <assert.h>



int drunlink(const char *filename)
{
return(!_unlink(filename));
}

int drrename(const char *oldfilename,const char *newfilename)
{
char	nfn[FNAMSIZ];
strcpy(drsplitpath(nfn, oldfilename),newfilename);
int ok=MoveFileEx(oldfilename, nfn, MOVEFILE_WRITE_THROUGH);
if (ok) return(YES);

char txt[FNAMSIZ+32];
sjhlog_file_error(oldfilename, strfmt(txt,"RENAME to %s",nfn));
SetErrorText("Error renaming %s to %s",oldfilename,nfn);
return(NO);
}

struct	DRSCN
	{
	int	    ct;
	char	path[FNAMSIZ];
	struct	_finddata_t ft;
	long	hdl;
	};

static int drcurdir(int drive, char *path)
{
char olddir[FNAMSIZ];
_getcwd(olddir,FNAMSIZ);
if(_chdrive(drive-'@')>=0 && _getcwd(path,FNAMSIZ)&& _chdir(olddir)==0)//_chdrive(old)>=0)
	{
	strdel(path,2);
	if (path[strlen(path)-1]!='\\') strcat(path,"\\");
	return YES;
	}
return(NO);
}

// Return current drive letter from the OS. Drive 1 -> 'A', Drive 2 -> 'B' etc
static int drcurdrv(void){return _getdrive()+'@';}


void setcwd(const char *path)
{
char pth[FNAMSIZ];
int drv=*_strupr(strcpy(pth,path));
if (drv<'A' || drv>'Z' || pth[1]!=':')
	m_finish("Bad cwd [%s]",path);
drcurdir(drv,&pth[2]);
}

char *drfullpath(char *path, const char *p)	// Return FULLPATH which NEVER ends in '\'
{
int	i=*p,j;
char	pth[FNAMSIZ];
if	((i=TOUPPER(i))!=0 && p[1]==':') p+=2; else i=drcurdrv();
*(long*)pth=i|(':'<<8);	// i.e. - drive letter followed by ':', then NULL
if (*p!='\\') drcurdir(i,&pth[2]);
strendfmt(pth,"%s\\",p);
while ((i=stridxs("\\.\\",pth))!=NOTFND) strdel(&pth[i],2);
while ((i=j=stridxs("\\..\\",pth))!=NOTFND)
	{
	while (pth[--j]!=':' && pth[j]!='\\') {;}
	strdel(&pth[j+1],i+3-j);
	}
while (pth[i=(strlen(pth)-1)]=='\\') pth[i]=0;
return(_strupr(strcpy(path,pth)));
}

char *drfulldir(char *fullpath, const char *path)	// Return FULLPATH which ALWAYS ends in '\'
{
if (strlen(drfullpath(fullpath,path))>3) strcat(fullpath,"\\");
return(fullpath);
}

// Parse 'fullpath' copying the directory part (incl. final '\') to 'dir_part
// Return pointer to the null byte at the end of 'dir_part', so app code can easily concatenate a different filename
#define FILESEPS ":/\\"
char  *drsplitpath(char *dir_part, const char *fullpath)
{
int	offset;
char	*filnam = (char*)fullpath;				
while	((offset=strinspn(FILESEPS,filnam)) != NOTFND) filnam += offset+1;	// step through separators to find last one
offset = filnam - fullpath;
memmove (dir_part, fullpath, offset);		// copy directory part to passed dir_part parameter
*(filnam=&dir_part[offset])=0;
return(filnam);							// return ptr to just after last separator
}

DRSCN *drscnist(const char *path)
{
DRSCN *scn = (DRSCN*)memgive(sizeof(DRSCN));
//?// kludge to accept network path starting with \\[computer]\[sharename]
if (path[0]=='\\' && path[1]=='\\')
	strcpy(scn->path,path); else
drfullpath(scn->path, path);
return (scn);
}

// SJH 16/06/05 Check returned filename. _findfirst/next return files that DON'T match search spec!!!
// Specifically, they return '.', '..', AND files with extra characters after the requested extension
static int filename_matches(const char *spec, const char *name)
{
int		si=strlen(spec),ni=0, sc, nc;
while (si && stridxc(spec[si-1],FILESEPS)==NOTFND) si--;

if (SAME2BYTES(name,".") || SAME3BYTES(name,".."))
	return(NO);

while (sc=spec[si], nc=name[ni], (sc || nc))
	{
	if (SAME4BYTES(&spec[si],"*.*"))
		return(YES);
	if (sc=='?')
		{
		si++;
		if (nc!='.')
			ni++;
		continue;
		}
	if (sc=='*')
		{
		si++;
		while (nc && nc!='.')
			nc=name[++ni];
		continue;
		}
	if (TOUPPER(sc)!=TOUPPER(nc))
		return(NO);
	si++; ni++;
	}
return(YES);
}

int drscnnxt(DRSCN *scn, FILEINFO *fi)
{
struct _finddata_t *ft=&scn->ft;

do	{
	if (scn->ct++) {if (_findnext(scn->hdl,ft)==NOTFND) return(NO);}
	else if ((scn->hdl = _findfirst(scn->path,ft))==NOTFND) return(NO);
	} while (!filename_matches(scn->path,ft->name));
fi->attr=(ushort)ft->attrib;
fi->dttm=(long)ft->time_write;
fi->size=ft->size;
strcpy(fi->name,ft->name);
return(YES);
}

void drscnrls(DRSCN *scn)
{
if (scn->hdl!=NOTFND)
	_findclose(scn->hdl);
memtake(scn);
}


int drinfo(const char *path, FILEINFO *pfile_info)
{
DRSCN *scn=drscnist(path);
int	ok=drscnnxt(scn,pfile_info);
drscnrls(scn);
return(ok);
}

int drattrget(const char *path, short *attr)
{
FILEINFO fi;
int ok=drinfo(path,&fi);
if (ok && attr) *attr=fi.attr;
return(ok);
}

	
/*
ulong drdsksp(char drive)
{
ULARGE_INTEGER
	FreeBytesAvailable,     // bytes available to caller
	TotalNumberOfBytes,     // bytes on disc
	TotalNumberOfFreeBytes; // free bytes on disc
int drv=TOUPPER(drive);
assert(drv>='A' && drv<='Z');
char dir[3]={drv,':',0};
if (GetDiskFreeSpaceEx(dir,&FreeBytesAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes))
	 return(FreeBytesAvailable.LowPart);
return(0);
}
*/

static char* get_filename_offset(char *path, const char *dflt_path)
{return(strend(strcat(drfullpath(path,*path?path:dflt_path),"\\")));}

void flnam_env(char *p, const char *fn, const char *envstr)
{
char *q=getenv(envstr);
if (!q && SAME4BYTES("TEMP",envstr) && !envstr[4]) q=getenv("TMP");
*p=0;
strcpy(get_filename_offset(p,q?q:""),fn);
}

int drisdir(const char *directory_path)
{
short attr;
char pth[FNAMSIZ];
drfullpath(pth,directory_path);
return(SAME2BYTES(&pth[1],":") || (drattrget(pth,&attr) && (attr&FA_DIR)));
}
