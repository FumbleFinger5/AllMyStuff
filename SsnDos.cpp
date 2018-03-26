#include "xlib.h"
#include "dll06.h"
#include "ssn.h"
#include "dgrp.h"
#include <stdio.h>


class SsnDos {
public:
SsnDos(void);						// Constructor used by app progs
SsnDos(HDL feod);					// Create database (handle to open EOD file passed)
~SsnDos() {Scrap(sn);dbsafeclose(db);};
void	GetSSN(DYNTBL **tbl);
DYNTBL	*GetSSNstage(SSNREC *sr);
short	glc2stg(int glc, char *srv, char depot);	// returns StageNo of 'glc' in Running Order, or NOTFND

private:
short	stg2idx(short stg);	// returns ordinal position of 'stg' in Running Order, or NOTFND
char	*stg2nam(short stg,char *name);	// Copy StgNam to str & Echo
char	*glc2nam(short glc,char *name);
int		get_glc(int mode, short *glc);
char	*srvnam_or_default(void);
void	import_eod(HDL feod);
int		ct;
SSN_DET	*sn;	// 2 x 'ct' short tables: StgNo, GlobalLocationCode
HDL		db,srv_btr,glc_btr;
SSN_GLC	g;
void	db_open(char *fn);
void	flush_sn(ushort *stg, ushort *glc);
int		dep,bd;
RHDL	rhdl;
struct	{char ver, fill[3]; RHDL srv_rhdl,glc_rhdl; char fill2[24];} hdr;
SSN_KEY	sv;
int		read_sn(RHDL r);
};

static	SsnDos	*ssn;


SsnDos::SsnDos(void)
{
db_open(findfile("SSN.DBF"));
}

SsnDos::SsnDos(HDL feod)
{
printf("Rebuilding SSN.DBF from %s\r\n",flnam(feod));
char fn[FNAMSIZ];
strcpy(fn,findfile("SSN.DBF"));
drunlink(fn);
if (!dbist(fn) || (db=dbopen(fn))==NULLHDL) goto err;
memset(&hdr,0,sizeof(hdr));
hdr.ver=2;
hdr.srv_rhdl=btrist(db,DT_USR,sizeof(sv));
hdr.glc_rhdl=btrist(db,DT_USHORT,sizeof(short));
dbsetanchor(db,recadd(db,&hdr,sizeof(hdr)));
dbsafeclose(db);
db_open(fn);
import_eod(feod);
printf("%d Services, %d Stages imported\r\n",btrnkeys(srv_btr),btrnkeys(glc_btr));
return;
err:
m_finish("Error creating %s",fn);
}

void EOD2SsnDos(HDL feod)	// Call the special constructor to rebuild SSN.dbf from EOD file
{
delete new SsnDos(feod);
}

void SsnDos::flush_sn(ushort *stg, ushort *glc)
{
int i, j;
if (!ct) return;
struct LKU {short stg, i;};
DYNTBL lku(sizeof(LKU), cp_short);
for (i=0;i<ct;i++) {LKU w={stg[i],i}; lku.put(&w);}
if (lku.ct!=ct) m_finish("Dup StageNo in %4.4s Running Order",sv.srv);
for (i=0;i<ct;i++)
	{
	j=lku.in(&stg[i]); if (j<0) m_finish("SysErr 15.8");
	sn->stg[j]=stg[i]; sn->stg[ct+j]=glc[i];
	}

char n1[128], n2[128];
MOVE2BYTES(n1,"?"); MOVE2BYTES(n2,"?");
if (get_glc(BK_EQ,&sn->stg[ct])) strcpy(n1,g.name);			// FIRST stage
if (get_glc(BK_EQ,&sn->stg[ct+ct-1])) strcpy(n2,g.name);		// LAST stage
while (YES)
	{
	int ln1=strlen(n1), ln2=strlen(n2);
	if (ln1+ln2+1<sizeof(sn->name)) break;
	if (ln1<ln2) n1[ln1-1]=0; else n2[ln2-1]=0;
	}
strfmt(sn->name,"%s-%s",n1,n2);
bkyadd(srv_btr,recadd(db,sn,sizeof(sn->name)+ct*4),&sv);
ct=0;
}

void SsnDos::import_eod(HDL feod)
{
int		i,section=0;
ushort	stg[256], glc[256];
char wrk[256], *srv, *p;
*(long*)sv.srv=ct=0;
sv.s.intro=c2bd("010100");
*(__int64*)sv.s.mask=NOTFND;	// "All depots"
sn=(SSN_DET *)memrealloc(sn,sizeof(sn->name)+256*4);
while (!fleof(feod))
	if (flgetln(wrk,sizeof(wrk),feod)>0)
		{
		if (!strcmp(wrk,"Section : Stage Definitions")) section='S';
		if (!strcmp(wrk,"Section : Route Definitions")) section='R';
		int comma=stridxc(COMMA,wrk);
		if (comma<1 || comma>4) continue;
		wrk[comma]=0;
		switch (section)
			{
			case 'S':
				if ((i=a2i(wrk,0))>0 && !a2err)
					{
					memset(&g,0,sizeof(SSN_GLC));
					strancpy(g.name,strtrim(&wrk[comma+1]),sizeof(g.name));
					bkyadd(glc_btr,recadd(db,&g,sizeof(SSN_GLC)),&i);
					}
				break;
			case 'R':
				comma=stridxc(COMMA,p=&wrk[comma+1]);
				if (comma<1) break;
				srv=srv2idx(wrk);
				if (!SAME4BYTES(srv,sv.srv))
					{
					flush_sn(stg,glc);
					MOVE4BYTES(sv.srv,srv);
					}
				if ((glc[ct]=a2i(p,0))>0 && (stg[ct]=a2i(&p[comma+1],0))>0)
					ct++;
				break;
			}
		}
flush_sn(stg,glc);
flclose(feod);
}


static int _cdecl cp_ssn_key(SSN_KEY *a, SSN_KEY *b)
{
int cmp=cp_mem4(a->srv,b->srv);
if (!cmp) cmp=cp_date_dep_scope(&a->s,&b->s);
return(cmp);
}

void SsnDos::db_open(char *fn)
{
//int prv=dbsetlock(YES);
if ((db=dbopen(fn))==NULLHDL) m_finish("Error opening %s",fn);
recget(db,dbgetanchor(db),&hdr,sizeof(hdr));						// Read anchor record
if ( hdr.ver<2			// version 2 format includes intro date in key
||  (srv_btr=btropen(db,hdr.srv_rhdl))==NULLHDL
||  (glc_btr=btropen(db,hdr.glc_rhdl))==NULLHDL)
	m_finish("Error reading %s",fn);
btr_set_cmppart(srv_btr,(PFI_v_v)cp_ssn_key);
bd=ct=0; dep=60; *(long*)sv.srv=0;
sn=(SSN_DET *)memgive(1);
//dbsetlock(prv);
}

short SsnDos::stg2idx(short stg)
{
return(ct?memidxw(stg,sn->stg,ct):NOTFND);
}

int SsnDos::get_glc(int mode,short *glc)
{
RHDL rhdl;
memset(&g,0,sizeof(SSN_GLC));
if (*glc==NOTFND || !bkysrch(glc_btr,mode,&rhdl,glc))
	return(NO);
recget(db,rhdl,&g,sizeof(SSN_GLC));
return(YES);
}

char* SsnDos::glc2nam(short glc,char *name)
{
get_glc(BK_EQ,&glc);
return(strcpy(name,g.name));
}

char* SsnDos::stg2nam(short stg,char *name)
{
int i=stg2idx(stg);
if (i!=NOTFND) i=sn->stg[ct+i];
return(glc2nam(i,name));
}


int SsnDos::read_sn(RHDL r)
{
int ct=0;
if (r)
	{
	int sz=zrecsizof(db,r)-sizeof(sn->name);
	if (sz>=4) ct=sz/4;
	}
sn=(SSN_DET *)memrealloc(sn,sizeof(sn->name)+ct*4);
if (r) recget(db,r,sn,sizeof(sn->name)+ct*4); else *sn->name=0;
return(ct);
}

DYNTBL* SsnDos::GetSSNstage(SSNREC *sr)
{
RHDL rh;
DYNTBL *tbl=new DYNTBL(sizeof(SSNSTG),cp_short);
if (!bkysrch(srv_btr,BK_EQ,&rh,&sr->k)) m_finish("Can't read Service/Stage record");
ct=read_sn(rh);	// (why can't I use the class variable here?
SSNSTG ss;
for (ss.seq=0; ss.seq<ct; ss.seq++)
	{
	ss.stg=sn->stg[ss.seq];
	stg2nam(ss.stg,ss.name);
	tbl->put(&ss);
	}
return(tbl);
}

char* SsnDos::srvnam_or_default(void)
{
static char name[sizeof(sn->name)];
strancpy(name,sn->name,sizeof(sn->name));
if (!*name)
	strfmt(name,"(%s)",srv2dsp_trim(sv.srv));	// default Servname
return(name);
}

void SsnDos::GetSSN(DYNTBL **tbl)
{
int		again=0;
RHDL	r;
*tbl=new DYNTBL(sizeof(SSNREC),(PFI_v_v)cp_ssnrec);
while (bkyscn_all(srv_btr,&r,&sv,&again))
	{
	SSNREC ssnrec;
	read_sn(r);
	MOVE4BYTES(ssnrec.k.srv,sv.srv);
	strancpy(ssnrec.name, srvnam_or_default(),sizeof(ssnrec.name));
	memmove(ssnrec.k.s.mask,sv.s.mask,8);
	ssnrec.k.s.intro=sv.s.intro;
	(*tbl)->put(&ssnrec);
	}
}





static void fmt_ssnrec(const SSNREC *s, char *str)
{
strfmt(str,"%s\t%s\t%s\t%s\t\n",
	   srv2dsp_trim(s->k.srv),dmy_stri(s->k.s.intro), s->name, mask2c(s->k.s.mask));
}


static void _GetSSN(char *ssnrec)
{
static DYNTBL *tbl=NULL;
if (ssnrec)
	{
	if (!tbl) (ssn=new SsnDos())->GetSSN(&tbl);
	if (return_up_to_64K(ssnrec, tbl, (PFV_v_c)fmt_ssnrec)) return;
	}
SCRAP(tbl);
}


// Return all records in proprietary database SSN.DBF
int __stdcall DllGetSSN(char *buf64)
{
int err=0;
DllLog z("DllGetSSN",&err,"co",buf64,0);
try
	{
	_GetSSN(buf64);
	}
catch (int e)
	{
	err=e;
	}
return(err);				// 0=No error
}

static void fmt_ssnstg(const SSNSTG *s, char *str)
{strfmt(str,"%d\t%s\t\n", s->stg, s->name);}


// populate binary format record from tab-delimited string
static SSNREC *a2ssnrec(SSNREC *s, const char *str)
{
MOVE4BYTES(s->k.srv,srv2idx(vb_field(str,0)));
s->k.s.intro=c2bd(vb_field(str,1));
strancpy(s->name,vb_field(str,2),sizeof(s->name));
c2mask(s->k.s.mask,vb_field(str,3));
return(s);
}

int __stdcall DllGetSSNstage(char *ssnrec, char *vbuf)
{
int err=0;
DllLog z("DllGetSSNstage",&err,"ci",ssnrec,"co",vbuf,0);
try
	{
	if (ssnrec)
		{
		SSNREC sr;
		DYNTBL *tbl=ssn->GetSSNstage(a2ssnrec(&sr,ssnrec));
		return_up_to_64K(vbuf, tbl, (PFV_v_c)fmt_ssnstg);			// We KNOW this can't overflow the 64K buffer, so no repeat call is needed
		delete tbl;
		}
	else SCRAP(ssn);
	}
catch (int e)
	{
	err=e;
	}
return(err);				// 0=No error
}

short SsnDos::glc2stg(int glc, char *srv, char depot)
{
if (!SAME4BYTES(sv.srv,srv))
	{
	memset(&sv,ct=0,sizeof(SSN_KEY));
	MOVE4BYTES(sv.srv,srv);
	RHDL rh;
	if (bkysrch(srv_btr,BK_GE,&rh,&sv) && SAME4BYTES(sv.srv,srv))
		ct=read_sn(rh);	// (why can't I use the class variable here?
	}
int i;
if (ct && (i=memidxw(glc,&sn->stg[ct],ct))!=NOTFND) return(sn->stg[i]);
return(NOTFND);
}

static int flappend(HDL op, char *fn)
{
HDL		f=flopen_search(fn);
if (f)
	{
	char	wrk[QRY_LINE_SZ];
	while (!fleof(f)) {flgetln(wrk,sizeof(wrk)-1,f); flputln(wrk,op);}
	flclose(f);
	}
return(f!=0);	// Return YES if passed filename existed and text therein was appended to 'op', else NO
}


static void unknown(char *srv, int glc)
{
static DYNTBL *t=NULL;
char	w[128], w2[16];
int		i, chg=YES;
DGRP	*g;
SRVi	*u, u1;
if (srv)
	{
	if (!t)
		{
		t=new DYNTBL(sizeof(SRVi),(PFI_v_v)cp_srvi);
		if ((g=read_dgrp("BADGLC",NO))!=NULL)		// If group file exists it was written by earlier runs of this process
			{										// There's only one category (membership is duff Srv+Stg combinations found before)
			for (i=0; i<g->dat->ct; i++)			// - so put previous errors into current table before we start adding any more
				{
				char *z=(char*)g->dat->get(i);		// Points to CatNum+SrvLo+SrvHI+StgLO+StgHi within the membership definition line
				MOVE4BYTES(u1.srv,&z[2]);
				u1.i=r2i(&z[10]);
				t->put(&u1);
				}
			scrap_dgrp(&g);
			}
		}
	MOVE4BYTES(u1.srv,srv);
	u1.i=glc;
	t->put(&u1);
	return;
	}
if (!t) return;	// If it's the shutdown call (srv=NULL), but we never added any duff stages to table, there's nothing to do here
HDL		fg=flopen_trap("BADGLC.TGF","w+"), fl=flopen_trap("BADGLC.LOG","w+");
flputln("BadGLC\r\nSB\r\n*Unknown",fg);			// (write "Resolved entity", "Dependency list", and "The one & only CategoryName" lines)
flputln(strfmt(w,"Undefined Global Location Codes importing ERG data %s\r\n",dmy_hm_str(calnow())),fl);
int indent;
for (i=0;i<t->ct;i++)
	{
	u=(SRVi*)t->get(i);
	char *s=strtrim(srv2dsp(u->srv));
	flputln(strfmt(w2,"%s,%d",s,u->i),fg);		// (write membership definition line to the group file)
	if (chg) indent=strlen(strfmt(w,"Service %-4.4s  Location Codes ",s));
	strendfmt(w,"%d",u->i);
	MOVE4BYTES(u1.srv,u->srv);
	chg=(i==t->ct-1 || !SAME4BYTES(u1.srv,u[1].srv));
	if (strlen(w)>120 || chg) {flputln(w,fl); strfill(w,indent,SPACE);}
	else strcat(w,",");
	}
flclose(fg);

flputln("\r\n\r\n",fl);
if (!flappend(fl, "_BadGLC.txt"))
	{
	flputln("If you continue, these codes will be loaded into the Ticket Archive unchanged.",fl);
	flputln("   Transcend will not show Stage Names for these locations, and will not be",fl);
	flputln("   able to place them within the Running Order for the associated Services.\r\n\r\n",fl);
	flputln("If you abandon the current job, the main Ticket Archive will NOT be updated.",fl);
	flputln("   Use ConfigEdit to import Stage definitions from an up-to-date EOD FareSet.",fl);
	flputln("   ERG ALTCNV ZIP file(s) input to the current job are listed in ERGALL06.LOG",fl);
	flputln("   You must copy them back to the input folder before retrying this process.",fl);
	flputln("\r\n\r\n   Note - this message can be replaced by site-specific text in _BADGLC.TXT",fl);
	}
flclose(fl);

SCRAP(t);
}



class SsnBOTH {
public:
SsnBOTH(void);
~SsnBOTH();
short	glc2stg(int glc, char *srv, char depot);
private:
SsnDos	*ssnD;
SSN		*ssnT;
};

static int stg_xlt=NOTFND;

SsnBOTH::SsnBOTH()
{
if (stg_xlt==NOTFND)
	{
	char wrk[128];
	if (*ti_var("REF2STG",wrk,TIV_UPPER)=='N') wrk[0]=0;
	if (!wrk[0] || wrk[0]=='Y' || wrk[0]=='T') stg_xlt=wrk[0];
	else m_finish("Invalid [REF2STG] (Y=SSN.DBF, T=TRANSCEND.MDB)");
	}
ssnD=NULL;
ssnT=NULL;
if (stg_xlt=='Y')
	if (drattrget(findfile("SSN.DBF"),0)) ssnD=new SsnDos();
	else if (drattrget(findfile("TRANSCEND.MDB"),0)) stg_xlt='T';
if (stg_xlt=='T') ssnT=new SSN(); 
}

SsnBOTH::~SsnBOTH()
{
SCRAP(ssnD);
SCRAP(ssnT);
unknown(NULL,NOTFND);
}


short	SsnBOTH::glc2stg(int glc, char *srv, char depot)
{
int stg=glc;
if (ssnD) stg=ssnD->glc2stg(glc,srv,depot);
if (ssnT) stg=ssnT->glc2stg(glc,srv,depot);
if (stg==NOTFND) unknown(srv,stg=glc);
return(stg);
}


int SsnDosStg2Idx(char *srv, int stg, char depot)
{
static SsnBOTH *sb=NULL;
if (!srv)
	{
	SCRAP(sb);
	}
else
	{
	if (!sb) sb=new SsnBOTH;
	if (stg) stg=sb->glc2stg(stg%10000,srv,depot);
	}
return(stg);
}

