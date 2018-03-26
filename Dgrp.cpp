#include "xlib.h"
#include "dgrp.h"
#include "xsq.h"
//?//#include "access.h"
//?//#include "TripA.h"
#include "ssn.h"

int	daynames;
int	count_occurence=NO;
char	*dop_schema;

DYNAG	*otkn;

//SY		*sy;  //?// this wasn't originally defined in this file!!!!


static char *valchar=" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ?";

static int dg_er,dg_ln;

static char *dg_txt[]={
	"",											// 0 (no error)
	"Invalid 'open mode' flag for Group File",	// 1
	"File not found",							// 2
	"Invalid Group Field name",					// 3
	"Invalid/Missing Field Dependency list",	// 4
	"Duplicate '+Other' Category definition",	// 5
	"No definition lines in Group File",		// 6
	"Line too long",							// 7
	"Missing 'include' file",					// 8
	"Missing Category name",					// 9
	"Bad Value/Range",							// 10
	"No definition lines for last Category",	// 11
	};

// Shuffle CCyy-mm-dd or dd/mm/yy into standard 6-char ddmmyy format
static void standardise_date(char *s)
{
int t=stridxc(9,s);
if (t>0) s[t]=0;
if (strlen(s)==10 && s[4]=='-' && s[7]=='-')
	{
	MOVE2BYTES(&s[0],&s[8]);
	MOVE2BYTES(&s[8],&s[2]);
	MOVE2BYTES(&s[2],&s[5]);
	MOVE2BYTES(&s[4],&s[8]);
	s[6]=0;
	}
else if (strlen(s)==8 && s[2]=='/' && s[5]=='/')
	{
	MOVE2BYTES(&s[2],&s[3]);
	MOVE2BYTES(&s[4],&s[6]);
	s[6]=0;
	}
}


// 'w' is a string to compare against either the 'original' (match=0),
// or 'revised' (match=1) substitute text values for this KeyField.
// If it matches an entry, return the OPPOSITE (revised or original)
// string from the substitution table, otherwise return original string
char *get_subst(SY *s, char *w, int match)
{
char *p[2];
if ((p[0]=s->subst)!=0)
	{
	int i=*(short*)p[0];
	for (p[0]+=2;i--;p[0]=&strend(p[1])[1])
		{p[1]=&strend(p[0])[1]; if (!strcmp(w,p[match])) return(p[!match]);}
	}
return(w);
}

static const char *vehquality="?KJL";	// NOT JKL!
char	vehicle_quality(char vehtyp)	// Convert VehTypeValue to scalar offset within quality
{
int offset=stridxc(vehtyp,vehquality);
return((offset<=0)?0:offset);
}

ulong c2k(char *p,SY *s)		// Passed ptr -> character string representing a value of datatype s->typ,
{								// returns long (maybe only int or char value) being the encoded value (or subscript)
int		i,j, typ=s->typ;
ulong	m=a2l(p,0);
int		ok=!a2err;
char	tmp[16];
DGRP	*g;
switch (typ)
	{
	case IX_ERGETM:
		m=hex2l(p,8);
		break;
	case IX_CTL:	case IX_RAWDATE:
		m=unformat_key(typ, p);
		break;
	case IX_CTLDATE:	case IX_SHDDATE:	case IX_DATE:	case IX_SONDTE:		case IX_SOFFDTE:
		standardise_date(strancpy(tmp,p,16));
		m=c2bd(tmp); if (tmp[6] || m<BD1980) goto err;
		p=tmp;
		break;
	case IX_DCD_TIME:
	case IX_TIME:		case IX_TKTTIM:		case IX_SONTIM: case IX_SOFFTIM:	case IX_SHDTIME:
	case IX_SHDEND:		case IX_TRPEND:		case IX_STGTIME:case IX_CTLTIME:	case IX_F6TIME:
	case IX_INS:		case IX_INS_END:
		if (p[4] || (i=a2hmv(p))<0) goto err;
		m=i;
		break;
	case IX_SRVPREFX:	case IX_SRVSUFFX:
		if (!SAME2BYTES(p,"0"))
			for (i=0;(j=p[i])!=0;i++)
				if (j<'A' || j>'Z')
					goto err;								// ELSE fall thru...
	case IX_CLASS:
		m=TOUPPER(*p); if (m<'A') m-='0'; else m-=55;
		if (m<0 || m>14) goto err;
		break;
	case IX_PRVTYPE:	case IX_WAYTYP:
		for (i=2;i--;)
			if ((j=stridxc(p[i],&valchar[1]))<0 || j>14+i) goto err;
			else if (i) m=j; else m|=(j<<4);
		break;
	case IX_SHDDUTY: case IX_SHDBUS:
	case IX_DUTY:		case IX_VEHICLE:	case IX_ETM:
		if (typ==IX_SHDDUTY) typ=IX_DUTY;
		if (typ==IX_SHDBUS) typ=IX_ETM;
		m=encode(typ,p);
		break;
	case IX_DAY:
		if ((m=day_bin(p))==0) goto err;
		break;
	case IX_BRDNAM: case IX_ALTNAM:
	case IX_CON:		case IX_REGNO:		case IX_POF:	case IX_RAWFILE:
		if (strlen(p)>(unsigned)s->maxl) goto err;
		m=s->stc->c2k(p);
		break;
	case IX_DOT:
		m=p[0] & 3;
		break;
	case IX_TIMGRP:		case IX_DAYGRP:		case IX_PRCGRP:
		m=*p;
		break;												// 1 byte schema values
	case IX_MODFIL:
		m=a2lx100(p);
		break;

	case IX_EV_TYP: case IX_CV_TYP:
		m=vehicle_quality(*p);
		break;

	default:
		if (ISDIGIT(typ) && (g=dgrp[typ-'0'])->flag&G_Dstr)
			for (m=ok=0;m<=s->maxv;m++)
				if (!strcmp(p,(char*)g->nam->get(m)))
					{ok=YES;break;}
		if (!ok) goto err;
	}
if (m<=s->maxv
&& strlen(p)<= (unsigned) s->maxl)
	return(m);
err:
//_sjhlog("bad fld %c:%d [%s]",typ,typ,p);
return(0xffffffff);
}


char *stc_txt(SY *s, ulong v)
{
char *p=s->stc->k2c(v);
if (s->typ==IX_BRDNAM || s->typ==IX_ALTNAM)
	for (int i=0;i<3 && *p;i++) p++;
if (strlen(p)>30)									// MAXIMUM LENGTH OF CHAR FORMAT OF ANY FIELD-ID = 30
	{
	sjhLog("Field %s (Alt%d) too long! [%s]",s->IXID,s->typ,p);
	static char dummy[32];
	return(strancpy(dummy,p,sizeof(dummy)));
	}
return(p);
}



static char*  typ_name(int kind, short code)
{
static char w[16];
char	*nam="";
return(nam);
}

char* tkt_name(ulong tpp)
{
static char str[TKTNAM_SZ];
int	a,t;
char	*s,wrk[64];
for (a=*wrk=0;a<=2;a++)
	{
	if (a) t=((a==1)?(tpp>>8):tpp)&0xFF; else t=tpp>>16;
	if (*(s=typ_name(a,t))) strendfmt(wrk,"%s%s",a?" ":"",s);
	}
return(strancpy(str,wrk,TKTNAM_SZ));
}

// Passed a pointer into sy[.] (from which we derive a datatype), and
// a 'ulong' (maybe only int or char in effective range) that is either
// an encoded value or table subscript of that datatype. Returns pointer
// to a string representation of the value (of fixed length) suitable
// for output on a print.
char *k2c(SY *s, ulong v)
{
static	int sw;
static	char str[4][Ix_MAXWID+1];
char	*p,*w=str[sw++&3];
int		typ=s->typ, len=s->maxl, istr, i;
if (typ==count_occurence) typ=0;			// Force (v) to default formatting as a number (it holds "occurence count")
switch (typ)
	{
	case IX_CLASSNAM:
		v<<=4;	// and fall thru...
	case IX_TKTNAM:
		strcpy(w,typ_name(0,(short)v));
		break;
	case IX_PAXNAM:
		if (s->flag&F_IsTxt) strcpy(w,stc_txt(s,v)); else
		strcpy(w,typ_name(1,(short)v));
		break;
	case IX_PAYNAM:
		strcpy(w,typ_name(2,(short)v));
		break;

	case IX_EV_TYP: case IX_CV_TYP:
		if (v>=strlen(vehquality)) v=0;
		*(short*)w=vehquality[v];
		break;

	case IX_LATEDEP:	case IX_LATEARR:
		strfmt(w,"%*d",len,(short)(v-10000));
		break;
	case IX_DAY:
		if (daynames) build_daysop(v,w);
		else strcpy(w,day_str(v));
		break;

	case IX_CTL:	case IX_RAWDATE:
		strcpy(w,ctl2c(v));
		break;
	case IX_ERGETM:
		strfmt(w,"%08lX",v);
		break;
	case IX_MODFIL:
		strcpy(w, lx1002c(v));
		break;
//	case IX_ERGFIL:	strfmt(w,"%08lX",v); break;

	case IX_JNYREC:
		v>>=16; 		// discard ID + fall thru for normal formatting
	case IX_JNYID:		case IX_JOURNEY:	case IX_PRVJNY:
		strfmt(w,"%4lu",v);
		break;

	case IX_PRVCLASSNAM:	case IX_PRVTKTNAM:
//		if (!otkn) m_finish("sys err 401");	// just let it fail at run time (must be a coding error)
		strcpy(w,(char*)otkn->get(v));
		break;

	case IX_PRVTYPE:	case IX_WAYTYP:	/////////////////////
		strfmt(w,"%02X",(short)v);
		break;

	case IX_CTLDATE:	case IX_DATE:		case IX_SONDTE:		case IX_SOFFDTE:case IX_SHDDATE:case IX_ORGDTE:
	case IX_F6DATE:
		k2c_cal(w,long_date(v));
		break;

	case IX_DAYGRP:
		if (daynames)	// if not, fall thru
			{
			if (!dop_schema) m_finish("Syserr 14");
			for (i=istr=0;i<7;i++)
				if ((unsigned)dop_schema[i]==v)
					istr|=(1<<i);
			build_daysop(istr,w);
			break;
			}

	case IX_DOT:
		strfmt(w,"%c","OI?!"[v&3]);		// !!!!! 3 should never occur!
		break;

	case IX_TIMGRP:	case IX_PRCGRP:	//case IX_TRNTYP:
		istr=v&255;		// make a trick 1-byte string
		strfmt(w,"%*.*s",len,len,&istr);
		break;

	case IX_CLASS: case IX_PRVCLASS:
		strfmt(w,"%X",(int)v);
		break;



	case IX_SHDDUTY: case IX_SHDBUS:
	case IX_DUTY: case IX_VEHICLE: case IX_ETM: case IX_DRIVER:
		if (typ==IX_SHDDUTY) typ=IX_DUTY;
		if (typ==IX_SHDBUS) typ=IX_ETM;
		strcpy(w,expand(typ,v));
		break;

	case IX_TIME:	case IX_TKTTIM:	case IX_SONTIM:	case IX_SOFFTIM:	case IX_SHDTIME:
	case IX_SHDEND: case IX_DCD_TIME:
	case IX_CTLTIME:case IX_STGTIME:case IX_F6TIME:	case IX_TRPEND:	case IX_INS:	case IX_INS_END:
		strcpy(w,hm_stri(v));
		if (typ==IX_TIME && v==1440) strcpy(w,"0000");
		break;

	case IX_WEEK:
		if (s->vbt=='D')
			{
			k2c_cal(w,long_date(week1+v*7-week_as_date));
			break;
			}				// ELSE fall thru
	case IX_BRDREC: v&=1023;		// get rid of the 'Sequence ID' part of Board No.
	default:
		if (typ && (s->flag&F_IsTxt)!=0)
			strcpy(w,stc_txt(s,v));
		else if (ISDIGIT(typ) && s->vbt=='C')		// i.e. dgrp[.].flag&G_Kstr
			strcpy(w,(char*)dgrp[typ-'0']->nam->get(v));
		else
			strfmt(w,"%*lu",len,v);
		break;
	}
strtrim(w);
if ((p=get_subst(s,w,0))!=w) strcpy(w,p);
if (s->maxp) k2c_pad(w,s);
return(w);
}

static void dgrp_dat_getfld1(SY *s, char *lo, char *hi, char* &d)
{
ulong v[2];
int ok=NO;
if ((v[0]=c2k(lo,s))!=0xffffffff && (v[1]=c2k(hi,s))!=0xffffffff)
	{
	switch (s->typ)
		{
		case IX_CON:	if (cp_con((char*)&v[0],(char*)&v[1])<=0) ok=YES; break;
		case IX_REGNO:	if (cp_reg((char*)&v[0],(char*)&v[1])<=0) ok=YES; break;
		case IX_POF:	if (cp_pof((char*)&v[0],(char*)&v[1])<=0) ok=YES; break;
		case IX_BRDNAM:	if (cp_bnm((char*)&v[0],(char*)&v[1])<=0) ok=YES; break;
		case IX_ALTNAM:	if (cp_anm((char*)&v[0],(char*)&v[1])<=0) ok=YES; break;
		case IX_SRVNAM:	if (cp_snm((char*)&v[0],(char*)&v[1])<=0) ok=YES; break;
		default:		if (v[0]<=v[1]) ok=YES; break;
		}
	}

if (!ok)
	{
	dg_er=10;	// bad value or range
	return;
	}

for (int i=0;i<2;i++)
	{
//?//	memmove_rv_sy(d,v[i],s);
	d+=s->len;
	}
}

static int fixable_ctl(char *p, int chr, int do_it)
{
if (do_it) p[0]=p[3]=chr;
return(p[0]==chr && p[3]==chr && p[6]==' ');
}

// If the field is IX_Ctl the string format contains dashes either side of 'month'.
// Toggle the 128-bit on these dashes so they don't get parsed as Range separators
int fix_ctl_str(char *p,int do_it)
{
int		chr[2]={'-','-'};	// Set both old/new char to '-'
chr[do_it]|=128;		// Change one of them to 128+'-'
int		len=stridxc(COMMA,p);
if (len==NOTFND) len=strlen(p);
if (!fixable_ctl(&p[4],chr[0], NO)) return(NO);
int		real_dash=stridxc('-',&p[11]);
if (real_dash>len-11) real_dash=NOTFND;
if (real_dash>0)
	if (fixable_ctl(&p[11+real_dash+5],chr[0], NO)) fixable_ctl(&p[11+real_dash+5],chr[1],YES);
	else return(NO);
fixable_ctl(&p[4],chr[1],YES);
return(YES);
}

static void dgrp_dat_getfld(SY *s, char* &p, char* &d)
{
char *lo,*hi;
int fix_ctl=((s->typ==IX_CTL || s->typ==IX_RAWDATE) && fix_ctl_str(p,YES));
int	sep=strinspn(",-",lo=hi=p);
if (fix_ctl) fix_ctl_str(p,NO);

if (sep!=NOTFND && p[sep]=='-') {hi=&p[sep+1];p[sep]=0;}
if ((sep=stridxc(COMMA,hi))==NOTFND) sep=strlen(hi); else hi[sep++]=0;
p=&hi[sep];
dgrp_dat_getfld1(s,lo,hi,d);
}

static void build_dgrp_dat(char *dat, char *fld, char *rg_buff)
{
int f=0;
SY *s;
for (char *p=rg_buff;!dg_er && fld[f];f++)
	{dgrp_dat_getfld(s=&sy[xlt[fld[f]]],p,dat); s->flag|=F_Eval;}
}


int flgetln_trim(char *s, int bufsz, HDL f)
{
if (fleof(f)) return(-1);
flgetln(s,bufsz,f);
return(strlen(strtrim(unquote(s))));
}

int flgetln_strip(char *str, HDL f)
{
int	c=flgetln_trim(str, QRY_LINE_SZ-1, f);
if (c>0)
	{
	if ((c=stridxc(SEMICOLON,str))!=NOTFND) {str[c]=0;strtrim(str);}
	c=strlen(str);
	}
return(c);
}


static int h_getln(HDL fp, char *h)
{
static HDL finc[10];			// These variables handle 'include' files without
static int fno=0;			// the calling routine needing to know about it.
int i;
HDL	f=fno?finc[fno-1]:fp;
h[0]=0;
while (fno || !fleof(f))
	{
	char wrk[QRY_LINE_SZ];
	if (fleof(f)) {flclose(f);f=finc[--fno];continue;}
	dg_ln++;
	if ((i=flgetln_strip(wrk,f))<0) continue;
	alt2chr(wrk);
	if (i>QRY_LINE_SZ-2) {dg_er=7;return(NO);}	// line too long
	strcpy(h,wrk);
	while ((i=stridxc(TAB,h))!=NOTFND) h[i]=' ';
	if (*h=='<')
		{
		char *n=flnam(f);
		if (++fno>=10 || (f=finc[fno-1]=flopen_search(&h[1]))==NULLHDL)
			{dg_er=8;return(NO);}	// can't open 'include' file
		continue;
		}
	if ((i=strlen(strtrim(h)))>0)
		if (h[i-1]=='_') *(h=&h[i-1])=0; else return(YES);
	}
return(NO);
}

int read_dgrp_cat(HDL f, char *dat, DGRP *g, char *rg_buff, int &ln, int &in_cat)
{
in_cat=(rg_buff[0]=='*');
if (!in_cat)
	if (g->other==NOTFND) g->other=ln;
	else return(5);						// duplicate 'other' category
char *catnam=&rg_buff[1];
if (strlen(catnam)>31)
	{
	catnam[31]=0;
	sjhLog("Truncating Category Name [%s] in %s",catnam,flnam(f));
	}
g->nam->put(catnam);*(short*)dat=ln++;
return(NO);	// no error
}

static char	*input;
static int read_dgrp1(DGRP *g, HDL f, char *rg_buff)
{
int i,ln,t,in_cat, fld;
char *dat;
dg_er=3;	// missing descriptor (derived Field Name)
if (h_getln(f,input)) g->desc=stradup(input); else return(NO);
dg_er=4;	// invalid dependency field
if (h_getln(f,input)) g->fld=stradup(input); else return(NO);
for (i=ln=0;(fld=input[i])!=0;i++)		// Save highest fldtype (least significant key)
	{
	t=xlt[fld];
	SY *s=&sy[t];
//	if (s->flag&F_OkKey)	// pre-300707
	if (s->flag&(F_OkKey|F_Tkx))
		{ln+=s->len;if (g->lsk<t) g->lsk=t;}
	else
		return(NO);	// Make sure its a valid keyfield
	}
g->lsk=sy[g->lsk].typ;
if ((g->flag&G_AddNam) && g->lsk!=IX_SRV) return(NO);
ln=2+ln*2;	// space for short holding 'Resolved Category Number', plus each fld has a 2-value range of size 'ln'
dat=(char*)memgive(ln);			// A work area to build each data def line in
g->dat=new DYNAG(ln);	// Allocate space in DGRP structure for datadef lines
g->nam=new DYNAG(0);		// And the vari-len string names for the groups
g->other=NOTFND;
dg_er=ln=in_cat=0;	// count to see if we get any definition lines at all
while (h_getln(f,rg_buff) && !dg_er)
	{
	strcpy(input,rg_buff);
	if (rg_buff[0]=='+' || rg_buff[0]=='*')
		{
		if ((dg_er=read_dgrp_cat(f,dat,g,rg_buff,ln,in_cat))!=0) break;
		}
	else
		{
		if (in_cat++) build_dgrp_dat(&dat[2],g->fld, rg_buff);
		else dg_er=9;	// definition line has no category name...
		if (dg_er) break;
		g->dat->put(dat);
		}
	}
if (!dg_er)
	{
	if (in_cat==1) dg_er=11;	// no definition lines for final category
	if (!g->dat->ct) dg_er=6;				// no definition lines at all!
	}
if (dg_er) return(NO);
if (g->other==NOTFND) {g->nam->put("Other");g->other=ln++;}
memtake(dat);
g->dat->put(0);g->nam->put(0);	// reduce to minimum size needed
return(YES);
}
// i

static char *findfile_ext(const char *fn, const char *ext)
{
static char fnam[FNAMSIZ];
default_extn(strcpy(fnam,fn),ext);
return(findfile(fnam));
}


void set_grp_max(DGRP *g, SY *s, int category_ct)
{
for (int j=s->maxv=(category_ct-1);j>=0;j--)
	{
	char *category_name=(char*)g->nam->get(j);
	set_maxc_str(&s->maxl,category_name);
	}
}


DGRP *read_dgrp(char *fnam, int must_exist)
{
int	i,c, flag=0, er=must_exist;
HDL	f;
DGRP	*g=NULL;
if ((i=stridxc(COMMA,_strupr(fnam)))!=NOTFND)	// G_Kstr/G_Dstr/G_Verify/G_AddNam/G_Stg2Idx
	for (fnam[i]=0;(c=fnam[++i])!=0;flag|=(1<<c))	// KDVAB = FlagBits 1/2/4/8/16
		if ((c=stridxc(TOUPPER(c),"KDVAB"))==NOTFND) {dg_er=1;goto err;}
char rg_buff[512];
input=&rg_buff[256];
fnam=findfile_ext(fnam,DEFAULT_GROUP_EXTN);
dg_er=2;				// file n/f
input[0]=dg_ln=0;
if ((f=flopen(fnam,"R"))!=NULLHDL)
	{
	g=(DGRP*)memgive(sizeof(DGRP));
	g->flag=flag;
	er=!read_dgrp1(g,f,rg_buff);
	flclose(f);
	}
if (!er)		// HERE is where we should sort the category membership definition lines
	{
//	list_dgrp(g);
	return(g);
	}
err:
char txt[128];
if (dg_ln>1) strfmt(txt," (Line:%d)\r\n[%s]",dg_ln,input); else *txt=0;
if (*dg_txt[dg_er]) strfmt(input," (%s)",dg_txt[dg_er]); else *input=0;
m_finish("Error %d%s reading group definition file %s %s",dg_er,input,fnam,txt);
return(0);
} // input


// Return ptr -> static copy of passed string after trimming leading/trailing spaces,
// and truncating at the first embedded space (if any) in the trimmed string.
// Mainly this is so we can ignore "derived text" such as Service / Driver / Etc Name when compressing a keyvalue
// Also handles leading/trailing spaces possibly passed by GUI (that uses them for dispay alignment on e.g. PaxTotal)
static char* keyfix(char *s)
{
static char	str[Ix_MAXWID+1];
strtrim(strancpy(str,s,Ix_MAXWID));
int spc=stridxc(' ',str);
if (spc!=NOTFND) str[spc]=0;
return(str);
}


// Each ETM TripDepartRecord is uniquely identified by ActivityDate+UniqueNo, where ActivityDate is Rollover-sensitive,
// and UniqueNo is the ordinal position of the Trip in that Day's Index, numbered from 0 upwards (i.e. - 0 = first Trip
// for the Day, as encountered by the indexing routine when reading Ticket Archive by ascending ModFile within Ctl-ID.
JNY_DET* vb2jd(char *vbjd, JNY_DET *jd)
{
static JNY_DET wjd;
while (*vbjd==' ') vbjd++;		// skip leading spaces
int	e=stridxc('+',vbjd);
if (e==NOTFND) m_finish("Bad JnyDet:%s",vbjd);
if (!jd) jd=&wjd;
jd->bd=a2i(vbjd,0);
jd->uniq=a2i(&vbjd[e+1],0);
return(jd);
}

static int grp_nam2sub(DYNAG *nam, char *s)
{
int i=nam->ct;
while (i--)
	if (!strcmp((char*)nam->get(i),s))
		return(i);
m_finish("SysErr 56.8");
return(NOTFND);
}

ulong unformat_key(int typ, char *str)
{
ulong	v;
switch (typ)
	{
	case IX_DATE:	case IX_WEEK:
		v=c2k(str,&SX(IX_DATE));
		if (typ==IX_WEEK) v=(v-week1)/7;
		break;
	case IX_DAY:
		v=daynam2num(str);
		break;
	case IX_BRDNAM:	case IX_ALTNAM:	case IX_CON:	case IX_REGNO:	case IX_POF: case IX_RAWFILE:
		v=SX(typ).stc->c2k(str);
		break;
	case IX_SHDDEP:	case IX_DEPNUM:	case IX_SONDEP:	case IX_SOFFDEP:
	case IX_JOURNEY:case IX_VALID_DCD: case IX_CONLINE:
	case IX_HDRERV:	case IX_HDRERX:
	case IX_MONTH:	case IX_YEAR:
		v=a2l(str,0);
		break;
	case IX_INS:	case IX_INS_END:	case IX_TRPEND:	case IX_TIME:case IX_SHD_TIME:	case IX_DCD_TIME:
		char w[4];
		MOVE4BYTES(w,str);
		if (w[2]==':') MOVE2BYTES(&w[2],&str[3]);
		v=a2hm(w);
		break;
	case IX_DRIVER:
		str=keyfix(str);	// fall thru
	case IX_SHDDUTY: case IX_SHDBUS:
	case IX_ETM:	case IX_VEHICLE:	case IX_DUTY:	case IX_MODULE:
		if (typ==IX_SHDDUTY) typ=IX_DUTY;
		if (typ==IX_SHDBUS) typ=IX_ETM;
		v=encode(typ,str);
		break;
	case IX_CTL:	case IX_RAWDATE:
		v=c2ctl(str);
		break;
	case IX_MODFIL:	case IX_REV:
		v=a2lx100(str);
		break;
	case IX_SHD_LINK:	case IX_PAX:
		v=a2l(keyfix(str),0);	// TO DO: ??Surely we don't need to use keyfix here??
		break;
	case IX_JNYNDX:
		v=*(long*)vb2jd(str,0);
		break;
	case IX_DOT:
		v=str[0] & 3;
		break;
	default:
		if (!ISDIGIT(typ))
			m_finish("Unsupported field (Alt%d)",typ);
		DGRP *g=dgrp[typ-'0'];
		if (g->flag & G_AddNam)
			str=keyfix(str);
		v=grp_nam2sub(g->nam,str);
		break;
	}
return(v);
}


// 'ip' either points to a single-char fld_id, or an (ALTnnn) sequence to be decoded into a fld_id using chr(nnn)
// OR it might be (xxxxxx), where xxxxxx is a Field-ID mnemonic that should match the IxID string in an SY[] entry.
// Copy whatever the fld_id character is to output area 's', and increment this pointer (it's passed by reference)
// Advance 'ip' past the single char (or bracketted text string), ready to be called again for next fld_id.
static void convert_one_fld_id(const char* &ip, char* &s)
{
int c=*(ip++);
if (c=='(')
	{
	const char *prv=ip;
	int i=0, err=0;
	while (i<3 && !err)
		{c=*(ip++); if (TOUPPER(c)!="ALT"[i++]) err=1;}
	if (!err)
		if ((i=stridxc(')',ip))==NOTFND || i>3 || (c=a2i(ip,i))<1 || c>255 || a2err) err=2;
	if (err)
		if ((c=get_defined_id(ip=prv,&i))!=0) err=NO;
	if (err) m_finish("ALT2STRING error");	// alt2chr
	ip+=(i+1);
	}
*(s++)=c;
}

// Passed 'context string' is a tab-separated list of 'field-id + value' elements, where
// field-id is either a single character or a (bracketted) string (AltNNN or id-mnemonic)
//
// This routine deals with THREE potential problems...
// 1.)	Trim leading/trailing spaces from field values
// 2.)	Discard 'supplementary text' (Serv/DriverName, etc.) from field values
// 3.)	Add a TAB after the LAST 'id + value' element if it's missing
//
// Returns an ALLOCATED string containing 'fixed' copy of the passed string
static char *delete_supplementary_text(const char *str)
{
DYNAG	d(0);
int i, sz=0;
while (*str)
	{
	char wrk[128];
	if ((i=stridxc(TAB,str))==NOTFND) i=strlen(strcpy(wrk,str));	// no TAB after last element
	else {if (i) memmove(wrk,str,i); wrk[i++]=0;}
	str+=i;
	if (*wrk!='(' || (i=stridxc(')',wrk))==NOTFND) i=0;		// Set 'i' to last char of field-id
	strtrim(&wrk[i+1]);										// Trim any associated 'value'
	if ((i=stridxs("  ",wrk))!=NOTFND) wrk[i]=0;			// Discard supplementary text after 2 spaces
	d.put(wrk);							// Add fixed element to temp dynag
	sz+=strlen(wrk);					// Add to total length of all fixed elements
	}
char *s=(char*)memgive(sz+d.ct+1);		// space for all elements + TAB for each + EOS byte
for (i=0;i<d.ct;i++)
	strendfmt(s,"%s\t",d.get(i));
return(s);
}

ALT2STRING::ALT2STRING(const char *pip, int with_values)
{
int	c,list=(stridxc(TAB,pip)==NOTFND);
char	*ip;
char	*o_ip;
char	ss[1024], *s=ss;

if (with_values) ip=o_ip=delete_supplementary_text(pip);
else ip=o_ip=stradup(pip);

if (*ip && list==with_values) sjhLog("Warning! - ALT2STRING %d:%d",list,with_values);

if (list && (c=strlen(ip))>0 && ip[c-1]==TAB) ip[c-1]=0;	// remove unwanted trailing tab on single string


// Is there a TAB in the string?
// If it's supposed to contain string values there should be, else NOT
if (*ip && ((stridxc(TAB,ip)!=NOTFND) != with_values))
	sjhLog("Warning! - ALT2STRING val=%d [%s]",with_values,ip);

while (*ip)
	{
	convert_one_fld_id((const char*&)ip,s);
	if (!list)					// then ip is a series of records, each Fld-ID + (strValue) + TAB
//		while ((c=*(s++)=*(ip++))!=TAB)				// TO DO: check (o/p sjh.log?) after old dart retires
		while (*ip && (c=*(s++)=*(ip++))!=TAB)		// 04/01/05 - kludge for OLD Dart not passing final TAB
			if (!c) m_finish("ALT2STRING error3");
	}
*s=0;		// append terminating EOS byte to converted string
converted=stradup(ss);
memtake(o_ip);
}



// Passed 'v' is a tab-separated list of VB format context, where first char of each entry is FldId,
// followed by text format value (the last entry is followed by a null byte).
// Returns "No. of Entries" formatted as CONTXT into 'Buff'
void str2Context_DYNAG(const char *strContext, DYNAG *tbl)
{
ALT2STRING as1(strContext, YES);
strContext=(const char*)as1.converted;
while (*strContext)
	{
	const char	*str;
	char	wrk[64];
	int		p=stridxc('\t',strContext)+1;
	if (p)
		{
		str=vb_field(strContext,0);
		strContext+=p;
		}		// TO DO: some calls from app don't put \t after final element in context
	else
		{
		str=strContext;
		strContext=strend(strContext);
		}
	CONTXT	c;
	c.typ=str[0];
	strtrim(strcpy(wrk,&str[1]));
	c.value=unformat_key(c.typ,wrk);
	tbl->put(&c);
	}
}

static	DGRP	*b2d,	// from DaysOp group file - get DayType for Date
				*b2h;	// from TermTime group file - get Term/Holiday Status of Date

// This is really just resolve_dgrp() without Curv[]...	// TO DO: make it use standard function
static char* resolve_b2(int bd,DGRP *b2)
{
int	i=b2->dat->ct;
struct RNGI {short grp; char lo[2],hi[2];} *d=(RNGI*)b2->dat->get(0);
while (i--)
	if (bd>=r2i(d[i].lo) && bd<=r2i(d[i].hi))
		return((char*)b2->nam->get(d[i].grp));	// Return member category name
return(0);
}

char *get_dop_special(int digit)
{
int i,ok;
char *n;
for (i=ok=0;!ok && i<b2d->nam->ct;i++)
	if (*(n=(char*)b2d->nam->get(i))-'0' == digit)
		ok=YES;
if (!ok)
	m_finish("Special DayType %d not defined in DAYSOP.TGF",digit);
return(&n[1]);
}


const char *dop2daynam(char dop)
{
if (dop&128)
	return(get_dop_special(dop&127));
return(sy_daynam[dopb2dop06(dop)]);
}


static int daysop_category(const char *n)
{
if (ISDIGIT(*n))
	return(128|(n[0]-'0'));
int i;
for (i=7;i--;)
	if (!_strnicmp(n,sy_daynam[i],strlen(n)))	// sy_daynm[0] is fixed as Sun (1=Mon, 2=Tue, etc.)
		break;
return(1<<i);
}

void get_dopterm(DOPTERM *dt, int bd)
{
char	*n;
dt->dop=DOPB(bd);						// Returns 1,2,4,... for Sun,Mon,Tue,...
if (b2d && (n=resolve_b2(bd,b2d))!=0)
	dt->dop=daysop_category(n);

if (b2h) dt->term=(resolve_b2(bd,b2h)?2:1); else dt->term=1;
}	// If Resolve_grp() returns 0 it's termtime, which we swap to 2
	// Else it'll return 1 (matched +Other), meaning it's School Hols

// Fill the 'b2d' structure with details of any Actual Calendar Dates
// that aren't to be processed as their normal Day of Week, and 'B2h'
// with all termtime start/end date ranges
void load_daysop(int do_it)
{
if (!do_it)
	{
	scrap_dgrp(&b2d);
	scrap_dgrp(&b2h);
	return;
	}
b2d=read_dgrp("DAYSOP", NO);
b2h=read_dgrp("TERMTIME", NO);
}



// Calling routine needs to process a Field-ID defined by a text string name,
// Loop thru all sy[] entries to see if there's one with this particular name
// If found, return the relevant Field-ID Type, otherwise 0 (unknown text name)
int get_defined_id(const char *str, int *len)
{
int	p=stridxc(')',str);
if (p>2 && p<30)			// There MUST be a closing bracket (caller already found an opening bracket)
	{
	char nam[32];
	SY *s=sy;
	for (strancpy(nam,str,p+1); s->typ; s++)
		if (!_stricmp(nam,s->IXID))
			{
			*len=p;
			return(s->typ);
			}
	}
return(0);
}


