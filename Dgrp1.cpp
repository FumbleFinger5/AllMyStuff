#ifdef DOCUMENTATION	//	DGRP1.CPP
						This module IS included in wxp02.dll, which has dedicated code to process DGRP's
						Module DGRP contains DGRP code that's NOT used by wxp02
#endif

#include "xlib.h"
#include "dll.h"
#include "dgrp.h"
#include "ssn.h"

char	*curv;
char	xlt[256];
char	ix_suppress[5];

int	rollover;		// Set to 180 by dllinit() (EQV 03:00) - may be overridden by TI.INI and/or *.QRY


DGRP	*dgrp[10];

int	date_fmt=-3;
int	wxp02_dll=NO;

int	dop_adj;

// Returns 1,2,3,... for Sun, Mon, Tue,...
int __stdcall DllFirstDayOfWeek(void)
{
return(dop_adj+1);
}

// Return complete list of all supported Field-ID's in this DLL
int __stdcall DllGetFieldList(char *str)
{
int	ret=0;
try
	{
	int typ;
	for (int i=0; (typ=sy[i].typ)!=0; i++)
		if (stridxc(typ,ix_suppress)==NOTFND)
			str[ret++]=typ;
	str[ret]=0;
	}
catch (int errcode)
	{
	ret=errcode;
	}
return(ret);
}

// Globally force all etm values to 0 for the field-id's passed.
int __stdcall DllSuppress(char *suppressed_fields)
{
int	err=0;
DllLog z("DllSuppress",&err,"ci",suppressed_fields,0);
try
	{
	char wrk[32];
	alt2chr(strancpy(wrk,suppressed_fields,sizeof(wrk)));
	strancpy(ix_suppress,wrk,sizeof(ix_suppress));
	}
catch (int errcode)
	{
	err=errcode;
	}
return(err);
}

#define WEEKSTART(d) ((((d)+11-dop_adj)%7)==0)	// TRUE for 1st day of week
short week_start(short bd)
{
while (!WEEKSTART(bd)) bd--;
return(bd);
}

void list_mnemonics(void)
{
int i,j;
SY	*s;
HDL f=flopen("FieldIDs.txt","w");
char str[255];
strfmt(str,"%s\t%s\t%s\t%s\t%s\t%s\t","Mnemonic","Name","char","ASCII","Length","Justification");
flputln(str,f);
for (i=0;(s=&sy[i])->typ!=0;i++)
	{
	*str=0;
	strendfmt(str,"(%s)\t",s->IXID);
	strendfmt(str,"%s\t",s->niam);
	strendfmt(str,"%c\t",ISALPHA(s->typ)?s->typ:'?');
	strendfmt(str,"(Alt%03d)\t",s->typ);
	strendfmt(str,"%d\t",s->maxl);
	j='C'; if (s->flag&F_RtJst) j='R'; else if (s->flag&F_LfJst) j='L';
	strendfmt(str,"%c\t",j);
	flputln(str,f);
	}
flclose(f);
}

int __stdcall DllGetFieldInfo(int typ, LPSTR buffer)
{
int	err=0;
DllLog z("DllGetFieldInfo",&err,"ii",typ,"co",buffer,0);
try
	{
	int i;
	SY *s=&SX(typ);
	*buffer=0;
	if (s->typ!=typ)
		{SetErrorText("Unknown Field-ID %c (%u)",typ,(char)typ); throw(SE_FLDID);}
	vb_append(buffer,"%s",s->niam);					// FIELD NAME
if (s->flag&F_Money) s->vbt='M';	// TODO - flakey!
	vb_append(buffer,"%c",s->vbt);					// VB DATATYPE
	i=0;   if (s->flag&(F_Money|F_Miles)) i=2;
	vb_append(buffer,"%d",i);						// DECIMALS
	i='C'; if (s->flag&F_RtJst) i='R'; else if (s->flag&F_LfJst) i='L';
	vb_append(buffer,"%c",i);						// JUSTIFICATION
	vb_append(buffer,"%d",s->maxp);					// LENGTH
	i='N'; if (s->flag&F_OkKey) i='Y';
	vb_append(buffer,"%c",i);						// SeqNo o/p after KeyValue (only used by dllGetNextRecord)
	i='N'; if (s->flag&F_OkDat) i='Y';
	vb_append(buffer,"%c",i);						// 04/06/09 - field is 'countable'
	}
catch (int errcode)
	{
	err=errcode;
	}
return(err);
}

int __stdcall DllGetFieldInfoEx(int typ, LPSTR buffer)
{
int	err=0;
try
	{
	DllGetFieldInfo(typ,buffer);
	SY *s=&SX(typ);
	vb_append(buffer,"(%s)",s->IXID);				// FIELD MNEMONIC
	}
catch (int errcode)
	{
	err=errcode;
	}
return(err);
}

static char*  next_one(char *n, int len)
{
static char h[4];
MOVE4BYTES(h,n);
while (len--) if (++(h[len])) break;
return(h);
}

// Return YES if value 'n' is matched by a range record for this typ
// If 'nxt' is passed, it's a 1/2/4 byte value as appropriate,
// and we'll set it to the value of the NEXT selectable value, if the
// current value 'n' isn't wanted (so caller can skip if possible).
int wanted_fld(char typ,char *n, char *nxt)
{
SY		*s=&SX(typ);
int	exct=s->exct;
int	len=sy_rng_len(s);		// Trick to cope with range on Rev, which is stored in only 4 bytes
int	len2=len*2, ct, cmp,ok;
char	*addr=s->rng;
if ((ct=s->inct)==0 && !exct) return(YES);	// No rng selections for fld - must be OK
if (typ==IX_DAY) return((n[0] & addr[0])!=0);
ok=!ct;	// No inclusions? - ok unless excluded...
if (nxt)
	if (ok) memmove(nxt,next_one(n,len),len);	// get next feasible into nxt
	else memset(nxt,255,len);		// assume no higher key is possible
PFI_v_v	cp_sy=s->Cp;
while (ct--)
	{												// 1:rng>n   -1:rng<n   0:rng==n
//short a0=r2i(addr), a1=r2i(&addr[len]), nn=r2i(n); if (nn>=1439) nn=nn;
	if ((cmp=(cp_sy)(addr,n))<=0 && (cp_sy)(&addr[len],n)>=0)
		{ok=YES;break;}
	if (nxt && cmp>0 && (cp_sy)(addr,nxt)<0) memmove(nxt,addr,len);
	addr+=len2;
	}
if (ok && (ct=exct)!=0)			// Check for excluded key ranges
	for (addr=&s->rng[s->inct*len2];ct--;addr+=len2)
		if ((cp_sy)(addr,n)<=0 && (cp_sy)(&addr[len],n)>=0) ok=NO;
return(ok);
}	// cp_rev len


static int _cdecl compare_subscripts(const char *a, const char *b, int typ)
{return(SX(typ).stc->compare_subscripts(*(short*)a, *(short*)b));}

static int _cdecl compare_subscripts_r2i(const char *a, const char *b, int typ)
{return(SX(typ).stc->compare_subscripts(r2i(a), r2i(b)));}


static int _cdecl cp_con_r2i(const char *a, const char *b) {return(compare_subscripts_r2i(a,b,IX_CON));}
static int _cdecl cp_reg_r2i(const char *a, const char *b) {return(compare_subscripts_r2i(a,b,IX_REGNO));}
static int _cdecl cp_pof_r2i(const char *a, const char *b) {return(compare_subscripts_r2i(a,b,IX_POF));}
static int _cdecl cp_bnm_r2i(const char *a, const char *b) {return(compare_subscripts_r2i(a,b,IX_BRDNAM));}
static int _cdecl cp_anm_r2i(const char *a, const char *b) {return(compare_subscripts_r2i(a,b,IX_ALTNAM));}
static int _cdecl cp_snm_r2i(const char *a, const char *b) {return(compare_subscripts_r2i(a,b,IX_SRVNAM));}

int _cdecl cp_con(const char *a, const char *b) {return(compare_subscripts(a,b,IX_CON));}
int _cdecl cp_reg(const char *a, const char *b) {return(compare_subscripts(a,b,IX_REGNO));}
int _cdecl cp_pof(const char *a, const char *b) {return(compare_subscripts(a,b,IX_POF));}
int _cdecl cp_bnm(const char *a, const char *b) {return(compare_subscripts(a,b,IX_BRDNAM));}
int _cdecl cp_anm(const char *a, const char *b) {return(compare_subscripts(a,b,IX_ALTNAM));}
int _cdecl cp_snm(const char *a, const char *b) {return(compare_subscripts(a,b,IX_SRVNAM));}




SY_TXT_CTL::SY_TXT_CTL(SY *s)
{
typ=s->typ;
sz=s->maxl+1;
Tbl=new DYNAG(0);
TblX=new DYNTBL(sz+4,(PFI_v_v)strcmp);
c2k("0");				// Force 1st entry to (n/a) in both tables (note - SX() is NOT valid at this point!)
}

SY_TXT_CTL::~SY_TXT_CTL()
{
delete Tbl;
delete TblX;
}

void SY_TXT_CTL::clear(void)	// Clear all except the 1st ("n/a") entry in both tables
{
while (Tbl->ct) Tbl->del(Tbl->ct-1);
while (TblX->ct) TblX->del(TblX->ct-1);
}


ulong SY_TXT_CTL::c2k(const char *s)
{
if (contract_end(s) || SAME2BYTES(s," ")) if (TblX->ct) return(0);


/////////////////////////////////// TO DO: this is messy!! It fixes SAX.qry when SrvName > 24 chars, but it's not properly worked thru!
char zz[128];
s=strancpy(zz,s,sz);
//////////////////////////////////

int p=TblX->in(s), item;

if (p!=NOTFND)
	{
	char *RecX=(char*)TblX->get(p);
	item=*(int*)&RecX[sz];			// Step over string + EOS byte to get (int) SubscriptNo
	}
else
	{
	char RecX[128];							// allow plenty of space for textstring+eos before subscript
	strancpy(RecX,s,sz);
	item=*(int*)&RecX[sz]=Tbl->ct;
	if (item) SX(typ).maxv=item;			// DON'T use SX() on first call, because Xlt[] hasn't been created!
	Tbl->put(RecX);
	TblX->put(RecX);
	}
return(item);
}

char* SY_TXT_CTL::k2c(ulong v)
{
return((char*)Tbl->get(v));
}


void SY_TXT_CTL::set_maxwidth(void)
{
SY *s=&SX(typ);
if (!(s->flag&F_IsOut) || !Tbl) return;
for (int i=s->maxl=0;i<Tbl->ct;i++)
	set_maxc_str(&s->maxl,k2c(i));
s->maxp=s->maxl;					// print column width must be at least wide enough for longest data value
set_maxc_str(&s->maxp,s->niam);		// (but make sure that's enough to accomodate the full field name from SY)
}


int SY_TXT_CTL::compare_subscripts(int a, int b)
{
int c=Tbl->ct; if (a>=c || b>=c) m_finish("SY_TXT_CTL::subscript error");
return(strcmp(k2c(a),k2c(b)));
}


static int etm_sq=NOTFND;

static char* esrt1(const char *c, char *s, int sq)
{
char	*e=strtrim(expand(IX_ETM,r2l(c)));
int	i, pos[2], len[2];
pos[0]=len[0]=pos[1]=len[1]=0;
if (etm_sq==1)
	{
	while (ISALPHA(e[len[0]])) len[0]++;
	len[1]=strlen(&e[pos[1]=len[0]]);
	while (e[pos[1]]=='0' && ISDIGIT(e[pos[1]+1]))
		{pos[1]++; len[1]--;}
	}
else len[0]=strlen(e);
memset(s,' ',12);
for (i=0;i<2;i++)
	memmove(&s[(i+1)*6-len[i]],&e[pos[i]],len[i]);
return(s);
}

int _cdecl cp_etm(const char *c1, const char *c2)
{
char s1[12],s2[12];
if (etm_sq==NOTFND)
	if ((etm_sq=ti_vari("ETMSEQ",0))>1 || etm_sq<0)
		m_finish("Invalid [ETMSEQ] setting");
return(memcmp(esrt1(c1,s1,etm_sq),esrt1(c2,s2,etm_sq),12));
}





// We've got a list of datatypes in 'fld' with a range for each in 'rng'
// - check each datatype's table range again current 'Curv' 
static int  grp_match(char *fld, char *rng)
{
int	f,typ,len;

for (f=0;(typ=fld[f++])!=0;rng+=(2*len))
	{
	SY		*s=&SX(typ);
	char	*cv=&curv[s->coset];		// Current value of this data item
	char	*rng_hi=&rng[len=s->len];	// (grab length of encoded value of this datatype, in passing)
	switch (typ)
		{
		case IX_DAY:
			if (!(rng[0] & cv[0])) return(NO);	// (must have bits set in common to match)
			break;
		case IX_BRDNAM: case IX_ALTNAM:
		case IX_CON:	case IX_REGNO:		case IX_POF:	case IX_RAWFILE:
			{					// Target & Ranges are 2-byte reversed subscripts to ConTract, etc
			int vc=r2i(cv);
			if (s->stc->compare_subscripts(r2i(rng),vc)>0 || s->stc->compare_subscripts(r2i(rng_hi),vc)<0) return(NO);
			break;
			}
		case IX_DOT:
			{
			if (((*rng & 3) > (cv[0] & 3)) || ((*rng_hi & 3) < (cv[0] & 3))) // Only compare lower 3 bits, so that 0x01 and "1" are the same
				return(NO);
			break;
			}	
		default:
			if (s->flag&F_Sign)
				{
				short rlo=r2i(rng), rhi=r2i(rng_hi), rcv=r2i(cv);
				if (rlo>rcv || rhi<rcv)
					return(NO);
				break;
				}
			if (memcmp(rng,cv,len)>0 || memcmp(rng_hi,cv,len)<0) return(NO);
		}
	}
return(YES);
}


// ?????????? ################################################################# ???????????????????????????????????????
// ?????????? this routine is slow when there are a lot of categories in the    ???????????????????????????????????????
// ?????????? group file. We should sort g->dat when it's first loaded, then do  ????????????????????????????????????
// ?????????? a binary chop search, rather than sequential step thru all items  ???????????????????????????????????????
// ?????????? ################################################################# ???????????????????????????????????????
// ?????????? ################################################################# ???????????????????????????????????????

// Figure out what the resolved match value is for this group, now that
// we know we've got all the relevant data values for testing in 'Curv'
int resolve_grp(DGRP *g)
{
int		i;
char	*dat;

for (i=0;(dat=(char *)g->dat->get(i))!=NULL;i++)
	{
	short category=*(short*)dat;
	if (!g->target || category==g->target-1)	// Only check if we match this membership line when there's no target,
		if (grp_match(g->fld,&dat[2]))			// or if this line's category matches the required target
			return(category);
	}
if (g->flag&G_AddNam)		// As at 16/10/02, g->lsk MUST BE 'S' if G_AddNam is set, which
	{						// means "resolve SrvGrp for unmatched entries as the input SrvNo"
	char *str=k2c(&SX(IX_SRV),r2l(&CURV(IX_SRV)));
	return(g->nam->in_or_add(str));
	}
return(g->other);
}

void scrap_dgrp(DGRP **g)
{
if (*g)
	{
	Scrap((*g)->fld);
	Scrap((*g)->desc);
	SCRAP((*g)->dat);
	SCRAP((*g)->nam);
	Scrap(*g);
	}
}

void scrap_dgrp10(void)
{
for (int i=0;i<10;i++)
	scrap_dgrp(&dgrp[i]);
}


int  set_rollover(char *hm)
{
rollover=0;	// (So A2hm() doesn't use the old value!)
rollover=a2hmv(hm);
for (SY *s=sy;s->typ;s++)
	if (s->flag&F_Time)
		s->maxv=BIGTIME;
return(rollover>=0 && rollover<360);
}

int	week_as_date;
int	week1;

int set_week1(char *b2)	// 020194 is Sunday
{
if (strlen(b2)>8) return(NO);
char	w[16];
int		param_includes_date=(strlen(strcpy(w,b2))>1);
if (param_includes_date)		// - if the parameter includes 'ddmmyy' Week1 StartDate
	{
	if (!SAME2BYTES(&b2[6],"D+"))
		{
		week1=c2bd(b2);				// Overide global variable
		if (calerr()) return(NO);	// Not a valid Date
		}
	strdel(w,6);				// 'eat' the optional date component and carry on parsing anything following
	}
if (TOUPPER(w[0])=='D')
	{
	week_as_date=(w[0]=='D')?7:1;				// set to 7 if upper case, 1 if lower case
	SY *s=&SX(IX_WEEK);
	s->maxp=6;									// Adjust characteristics of SY control settings for Ix_WEEK field-id
	s->vbt='D';
	strdel(w,1);								// 'eat' the D/d character
	if (param_includes_date && w[0]=='+')		// Can only overide Week1 if ddmmyy WAS specified on param
		{dop_adj=DOP(week1); strdel(w,1);}		// change Dop_adj to suit defined weekStart, and eat the character
	}
else if (!param_includes_date) return(NO);		// extraneous char, or ?W with no sub-params at all
if (w[0]) return(NO);							// Error if any 'uneaten' param chars left
if (DOP06(week1))								// Error if week1 isn't FirstDayOfWeek
	{
	week1+=dop_adj;
//	SetErrorText("Date is %s not First Day of Week (%s)",sy_daynam[DOP06(week1)],sy_daynam[dop_adj]);
//	return(NO);
	}
return(YES);	// If we get here, param is OK
}


char rotate_7bits(char flag, int shift)
{
while (shift--)
	flag=((flag>>1)|((flag&1)?64:0));
return(flag);
}

int _cdecl cp_daysop(const char *a, const char *b)
{
return(cp_short_v(rotate_7bits(*a,dop_adj), rotate_7bits(*b,dop_adj)));
}


void sy_init(void)
{
if (curv)		// - then we've been called before
	memtake(curv);
else			// First call (some dll's call this more than once)
	{
	dop_adj=ti_vari("WEEK_START",0)%7;
	week1=c2bd("070190")+dop_adj;
	}
int	i,j;

SY *s=sy;
for (i=j=0;s->typ;i++)
	{
	if (wxp02_dll)
		{
		if (s->typ==IX_REV) s->len=5;
		if (s->typ==IX_PAX) s->len=4;
		}
	if (s->vbt=='D') s->maxl=10;
	s->coset=j;
	j+=s->len;
	s++;
	}
curv=(char *)memgive(j);		// current values area (used by Group Lookup)
memset(xlt,i,256);				// makes unused xlt[.] entries force errors
for (i=0;sy[i].typ;i++) xlt[sy[i].typ]=i;	// set xlt[.] for valid fieldtypes

char wrk[TEXT_LINE_SZ];
set_rollover("0300");			// Initialise sy[.].maxv for F_Time
if (*ti_var("ROLLOVER",wrk,0))	// If overiding system default
	set_rollover(wrk);			// then re-initialise sy[.].maxv
}


// This routine moves an sy[.] entry up to a higher position in the array, and adjusts Xlt[.] to reflect the change.
// It's used when a Category has been defined, to reposition the category's sy[.] entry to immediately after the least
// significant key used in  the category definition (so we know to resolve the category value as soon as that's known).
// ALSO used to move the Timeband IX_TIMGRP entry to follow the particular Fld-ID (Jny, Stage, or Tkt) defining Timeband.
static void  sy_move1(int to, int from)
{
SY w;
memmove(&w,&sy[from],sizeof(SY));		// Save the grp sy[.] entry
memmove(&sy[to+1],&sy[to],(from-to)*sizeof(SY));	// shuffle up sy[.]
memmove(&sy[to],&w,sizeof(SY));		// Recover the grp sy[.] entry
for (to=0;sy[to].typ;to++) xlt[sy[to].typ]=to;	// reset xlt[.] for valid types
}

void  sy_move(int to, int from)
{
SY *s;
int fld;
do (sy_move1(to++,from++));
	while ( ((s=&sy[from])->flag&F_Eval)!=0 && ((fld=s->typ)==IX_TIMGRP || ISDIGIT(fld)));
}

void sy_move_timgrp(int tb_basis)		// Move sy[.] entry for Field-ID Ix_TIMGRP up to the slot
{										// immediately after (JnyTime/BrdStgTime/IssTime, as reqd).
static char fld[]={IX_TIME,IX_STGTIME,IX_TKTTIM};
int i=xlt[fld[tb_basis]];				// Which sy[.] gives timeband? (0/1/2=Jny/Brd/Tkt)
sy[i].flag|=F_Eval;						// (we NEED to evaluate the preceding field)
sy_move(i+1,xlt[IX_TIMGRP]);			// - now move Ix_TIMGRP fld so it comes just after (j)
}


void  k2c_cal(char *w, long cal)
{
char	*fmt="%02D%02O%02Y";
switch (date_fmt)
	{
	case 1: fmt="%04C%02O%02D";break;
	case 2: fmt="%3.3M%02D";break;
	case 3:	bd2vb(short_bd(cal),w); return;
//,	case 3: fmt="%04C-%02O-%02D";break;
	}
calfmt(w,fmt,cal);
}

void  set_date_fmt(void)
{
char	fld[4]={IX_WEEK,IX_DATE,IX_SHDDATE,0};
char	wrk[32];
int		i=0;
date_fmt=-date_fmt;
if (SX(IX_WEEK).vbt!='D') i=1;	// Skip first field if WeekNo is to be output as a Number, not a Date
while (fld[i]) {k2c_cal(wrk,0); SX(fld[i++]).maxl=strlen(wrk);}
}

// return the (char, int, or long) value at 'ad', and increment address
ulong  fld_getl(char* &ad, int len)	// ('ad' passed by REFERENCE!)
{
ulong n=r2long(ad,len);
ad+=len;
return(n);
}

void	dop_verify(void)
{
SY *s=&SX(IX_DATE), *sydop=&SX(IX_DAY);
if ((SX(IX_WEEK).flag&F_Eval) && !week1)
	if (s->inct)
		{
		int i=r2i(s->rng);
		week1=i-DOP06(i);
		}
	else
		m_finish("Week1 Start Date not available");

if (sydop->inct)
	{
	int		dop=*sydop->rng, ct=s->inct+s->exct, bd1,bd2, ok=0, len=s->len;
	if (ct)
		{
		char	*ad=s->rng;
		while (ct--)
			for (bd1=fld_getl(ad,len),bd2=fld_getl(ad,len);bd1<=bd2;bd1++)
				if (ct<s->inct) ok|=DOPFLAG(bd1);
				else ok&=~DOPFLAG(bd1);
		if (!(ok&dop)) m_finish("Days of Operation parameter excludes all requested Dates!");
		}
	}
}

void k2c_pad(char *w, SY *s)
{
int i;
int RtJst=s->flag&F_RtJst, LfJst=s->flag&F_LfJst;
for (i=0;strlen(w)< (unsigned) s->maxp;i=!i)
	if (LfJst || (i && !RtJst)) strcat(w," "); else strinsc(w,' ');
}

void  alt2chr(char *s)
{
int i,c;
while ((i=stridxc('(',s))!=NOTFND)
	{
	int ok=NO;
	s+=(i+1);
	for (i=0;i<3;i++) if (TOUPPER(s[i])!="ALT"[i]) break;
	if (i==3 && (i=stridxc(')',s))>=4 && i<=6 && (c=a2i(&s[3],i-3))>=1 && c<=255 && !a2err)
		ok=YES;
	if (!ok)
		{
		if ((c=get_defined_id(s,&i))!=0)
			ok=YES;
		}
	if (ok)
		{
		strdel(s--,i+1);
		*s=c;
		}
	}
}

int  flgetln_alt2chr(char *str, char *org, HDL f)
{
int		i, len;
char	*s;
*(s=str)=0;
while ((len=flgetln(org,QRY_LINE_SZ-1,f))>0)
	{
	if (len>=QRY_LINE_SZ-2)
		{
		SetErrorText("Line too long");
		return(NOTFND);
		}
	alt2chr(strcpy(str,org));
	if ((i=stridxc(SEMICOLON,str))!=NOTFND) str[i]=0;
	while ((i=stridxc(TAB,str))!=NOTFND) str[i]=' ';
	len=strlen(strtrim(str));
	if (len)
		if (str[len-1]=='_') *(str=&str[len-1])=0;
		else break;
	}
return(s[0]!=0);
}



int can_include=YES;
int  getln_buff(HDL fp, char *buffer)
{

static HDL f,finc[11];	// These variables handle 'include' files without
static int fno=0;		// the calling routine needing to know about it.
static char qrytxt[QRY_LINE_SZ];
	
if (!fp)
	{
	strcpy(buffer,"Parameter error ");
	while ((f=finc[fno])!=0)
		{
		strendfmt(buffer,"in %s\r\n",flnam(f));
		finc[fno]=0;
		flclose(f);
		if (fno) fno--;
		}
	strendfmt(buffer,"Param: %s\r\n",qrytxt);
	DllGetLastError(strend(buffer));
	m_finish("%s",buffer);
	}
if (!fno) finc[0]=fp;
*buffer=0;
f=(fno && can_include)?finc[fno]:fp;
while ((fno && can_include) || !fleof(f))
	{
	if (fleof(f))		// We're only handling eof internally for 'include' files, not the first one, so here fno>0
		{flclose(f);if (--fno) f=finc[fno]; else f=finc[fno=0]=fp; continue;}
	if (flgetln_alt2chr(buffer,qrytxt,f)<0)
		getln_buff(0,buffer);
	if (*buffer=='<')
		{
		if (fno>9 || (finc[fno+1]=flopen_search(&buffer[1]))==NULLHDL)
			{
			SetErrorText("Can't open Include file");
			getln_buff(0,buffer);
			}
		f=finc[++fno];
		}
	else if (*buffer) return(YES);
	}
return(NO);
}



static double datfld_val1(char *ad, int len)
{
switch (len)
	{
	case 1: return(*ad);
	case 2: return(*(short*)ad);
	case 5: return(100.0*(*(long*)ad)+(signed char)ad[4]);
//	case 8: return(*(double*)ad);	// (support 'rev' as double)
	default: return(*(long*)ad);
	}
}

double datfld_val(SY *s,char *ad)
{
return(datfld_val1(&ad[s->koset], s->len));
}

static int _cdecl cp_rev (const char *a, const char *b)
{
int cmp=cp_long(a,b);
if (!cmp) cmp=cp_short_v(a[4],b[4]);
return(cmp);
}

void setup_cp(void)
{
for (SY *s=sy; s->typ; s++)
	switch (s->typ)
		{
		case IX_DAY:	s->Cp=(PFI_v_v)cp_daysop;	break;
		case IX_REV:	s->Cp=(PFI_v_v)cp_rev;		break;
		case IX_ETM:	s->Cp=(PFI_v_v)cp_etm;		break;
		case IX_REGNO:	s->Cp=(PFI_v_v)cp_reg_r2i;	break;
		case IX_POF:	s->Cp=(PFI_v_v)cp_pof_r2i;	break;
		case IX_CON:	s->Cp=(PFI_v_v)cp_con_r2i;	break;
		case IX_BRDNAM:	s->Cp=(PFI_v_v)cp_bnm_r2i;	break;
		case IX_ALTNAM:	s->Cp=(PFI_v_v)cp_anm_r2i;	break;
		case IX_SRVNAM:	s->Cp=(PFI_v_v)cp_snm_r2i;	break;
		default:
			switch (s->len)
				{
				case 1: s->Cp=(PFI_v_v)cp_mem1;		break;
				case 2: s->Cp=(PFI_v_v)cp_mem2;		break;
				case 4: s->Cp=(PFI_v_v)cp_mem4;		break;
				default: m_finish("System error 84");
				}
			break;
		}
}


char *op_key;		// List of key flds and offsets in opk elem

int _cdecl cp_opk(const char *a, const char *b)
{
int cmp,i, p, fld;
SY	*s;
for (i=p=cmp=0;!cmp && (fld=op_key[i++])!=0;p+=s->len)
	{
	s=&SX(fld);
	if (!s->Cp) m_finish("system error 82");
	cmp=(s->Cp)(&a[p],&b[p]);
	}
return(cmp);
}


// TO DO: Can I figure out a more elegant way to make these sort comparators use a 3rd param?
struct DGRP_SRT_CTL
	{
	PFI_v_v cp;
	DYNAG	*d;
	} dgsc[10];

static int _cdecl cp_dgrp_common(const char *a, const char *b, int i)
{
short ai=r2i(a), bi=r2i(b);
DYNAG *d=dgsc[i].d;
char *aa=(char*)d->get(ai), *bb=(char*)d->get(bi);
return((dgsc[i].cp)(aa,bb));
}

static int _cdecl cp_dgrp0(const char *a, const char *b) {return(cp_dgrp_common(a,b,0));}
static int _cdecl cp_dgrp1(const char *a, const char *b) {return(cp_dgrp_common(a,b,1));}
static int _cdecl cp_dgrp2(const char *a, const char *b) {return(cp_dgrp_common(a,b,2));}
static int _cdecl cp_dgrp3(const char *a, const char *b) {return(cp_dgrp_common(a,b,3));}
static int _cdecl cp_dgrp4(const char *a, const char *b) {return(cp_dgrp_common(a,b,4));}
static int _cdecl cp_dgrp5(const char *a, const char *b) {return(cp_dgrp_common(a,b,5));}
static int _cdecl cp_dgrp6(const char *a, const char *b) {return(cp_dgrp_common(a,b,6));}
static int _cdecl cp_dgrp7(const char *a, const char *b) {return(cp_dgrp_common(a,b,7));}
static int _cdecl cp_dgrp8(const char *a, const char *b) {return(cp_dgrp_common(a,b,8));}
static int _cdecl cp_dgrp9(const char *a, const char *b) {return(cp_dgrp_common(a,b,9));}

PFI_v_v dgsc_cp[10]=
	{
	(PFI_v_v) cp_dgrp0,
	(PFI_v_v) cp_dgrp1,
	(PFI_v_v) cp_dgrp2,
	(PFI_v_v) cp_dgrp3,
	(PFI_v_v) cp_dgrp4,
	(PFI_v_v) cp_dgrp5,
	(PFI_v_v) cp_dgrp6,
	(PFI_v_v) cp_dgrp7,
	(PFI_v_v) cp_dgrp8,
	(PFI_v_v) cp_dgrp9
	};


// After the accumulator table (or temp btree) has been filled by load(), it's sorted into the 'final'
// sequence using cp_opk, which uses each KeyField's particular comparator function from SY table.

// The default comparator for Group Fields sorts by Category Number within the associated dgrp DYNAG
// This routine checks for any Group Fields with the G_Addnam flag set, and changes the comparator to
// one that sorts by Category Name.

// NOTE (Dec 2005) - G_Addnam only currently used in Group fields where Least Significant Dependency Field
// is IX_Srv, where the comparator assumes Category Names are amenable to sorting as ServNo's. This is ok
// if a Category is called, for example, "70-79", because the Cp_srv() comparator truncates KeyValues
// at the [position where it meets a character not valid for ServNo, so that key sorts as "70"
void set_dynag_cp(void)
{
for (int i=0;i<10;i++)
	if (dgrp[i] && (dgrp[i]->flag&G_AddNam))
		{
		SX('0'+i).Cp=dgsc_cp[i];
		dgsc[i].d=dgrp[i]->nam;
			dgsc[i].cp=(PFI_v_v)strcmp;
		}
}


// Trick to cope with range on Rev, which is stored in only 4 bytes (not 5, as used in the main accumulator table)
int sy_rng_len(SY *s)
{
if (s->len==5) return(4);
return(s->len);
}


DGRP *find_depot_tgf(void)
{
for (int i=0;i<10;i++)		// (or any *.tgf with dependency DepNum) telling us which 'depot' (Operator) we're reporting on.
	{
	DGRP	*g=dgrp[i];
	if (g && *(short*)g->fld==IX_DEPNUM && g->target)
		return(g);
	}
m_finish("Query must be for a single Operator");
return(NULL);
}

int find_depnum(DYNAG *d, int target)
{
for (int j=d->ct; j--; )
	{
	char *dat=(char *)d->get(j);
	if (*(short*)dat==target && dat[2]==dat[3])
		return(dat[2]);
	}
m_finish("SysErr 68.1");
return(0);
}
