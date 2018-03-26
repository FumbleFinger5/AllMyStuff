#define DEFAULT_GROUP_EXTN "TGF"

extern	int	wxp02_dll;	// TRUE when using wxp02

#define Ix_MAXWID 40

struct SY; //?// predeclare

class SY_TXT_CTL {
public:
SY_TXT_CTL(SY *s);					// Create an SY text field control object
~SY_TXT_CTL();						// Release
ulong	c2k(const char *p);			// Passed ptr -> character value of 'typ', return binary stored value
char	*k2c(ulong v);				// (since this is internal to SY_Txt_Ctl, it doesn't need the 'SY' param)
void	set_maxwidth(void);
int		compare_subscripts(int a, int b);
void	clear(void);
//void	list(void);
DYNAG	*Tbl;
private:
int		typ,		// The field-id for which this instantiation is managing text strings
		sz;			// Maximum possible length of string, including terminating null-byte
DYNTBL	*TblX;
};

char *stc_txt(SY *s, ulong v);


// These are the bitflags in sy[.].flag for each field:-
#define F_OkKey 0x0001	// Can be a Key output for QUERIES
#define F_OkRng 0x0002	// Can be a Range input for	QUERIES
#define F_IsOut 0x0004	// Is a key or data field for output
#define F_OkScm 0x0008	// Can be a Schema
#define F_OkDat 0x0010	// Can be output as Data
#define F_OpAll 0x0020	// O/p ALL possible values even if no matching data
#define F_IsTxt 0x0040	// This is a text field with its own DYNAG of "strings that have occured" (actual values are subscripts into this)
#define F_RtJst 0x0080	// Right-justify field on output
#define F_Miles 0x0100	// (as 1/10'ths)
#define F_Eval  0x0200	// Needs to be evaluated
#define F_Invrt 0x0400	// Reverse output sort order
#define F_LfJst	0x0800	// Left-justify field on output
#define F_Time	0x1000	// This is a TIME field - maxv is BIGTIME (Rollover-sensitive)
#define F_Money	0x2000	// This is a Money field (count to nearest penny)
#define F_Date	0x8000	// This is a Date field
#define F_Sign	0x10000	// This field can be -ve
#define F_Tkx	0x20000	// Field is in INDEXES
#define F_Trp	0x40000	// Field is in TRIP archive
#define F_Tkt	0x80000	// Field is in TICKET archive
#define F_Jst	(F_LfJst|F_RtJst)		// Either justification flag
#define F_DecPt	(F_Miles|F_Money)		// (both have decimal point)
#define F_ISDAT	(F_IsOut|F_OkDat)

// These are the bitflags in a Dgrp.flag for a category:-
#define G_Kstr		1		// Convert output to string (matching entry name)
#define G_Dstr		2		// Interpret input ranges as strings
#define G_Verify	4		// Check for overlaps in data definitions
#define G_AddNam	8		// Add sought lsk as name if resolution=Other
#define G_Stg2Idx	16		// Convert StageNo's to 'ordinal within *.ssn

#define SX(t) sy[xlt[t]]
#define CURV(t) curv[SX(t).coset]

#define BIGTIME	(1440+rollover-1)	// (1440+ default Rollover)
#define SQ6xZ	2176782335L			// 6 Z's in base 36	\_ (Looked at as a
#define SQ4xZ	1515870810L			// 4 Z's as chars	 _ 4-char c2l value)
#define SQ3xZ	(SQ4xZ)				// 3 Z's as chars	/
#define	TKTNAM_SZ 20		// Max length of TicketNam (TktTyp+PaxTyp)

struct SY	{
		char	*niam;		// Name of element
		char	*IXID;		// This is the 'xxxxx' part of 'IX_xxxxx' for this field-ID
		ulong	maxv;		// Maximum value (as Numeric) when encoded (Max actual value for data fields)
		uchar	typ;		// 1-char typ identifier
		char	vbt,		// VB datatype ***ADDED 04/07/02***
				len,		// Length of encoded value
				maxl,		// Max length of character form of value
				sdigits,	// No. of digits needed for subscript values
				maxp;		// No. of chars for print column - see newnames()
		ushort	inct,		// Number of INCLUDE range parameters for typ
				exct;		// Number of EXCLUDE range parameters for typ
		char	koset;		// offset in opk of element
		short	coset;		// offset in Curv of current value of element
		char	*rng,		// ptr-> 1st selection range (if any) in syd[]
				*uvals,		// List of unique values found for key
				*subst;		// List of 'substitute output value' texts
		SY_TXT_CTL *stc;	// Control object for storing variable-length string values in DYNAG
		int	nvals;		// Number of different opk[] values of element
		PFI_v_v Cp;			// Comparator function for this field
		int	flag;
				};


extern	SY		*sy;

extern	int		week1;

extern	char	*curv;

extern	char	xlt[];		// 256 x 'subscript into SY table' as a quick way to reference the SY entry for any given field-id
							// SY holds field-id's in the order current process resolves them (only field-id's that it actually CAN resolve)

struct DGRP {
	char	*fld,		// list of the key types making up a definition line
			*desc,		// Description of this Group (group file 1st line)
			lsk,		// Datatype of the least significant subsiduary key
			flag;		// 1=Convert Key, 2=Convert Data (to text eqv)
						// 4=Verify Rngs, 8=Resolve +Other as lsk curvalue
	short	target;
	int		other;		// Subscript of 'Other' category (usually fst/last)
	DYNAG	*dat;		// Array of the input keymatch lines
	DYNAG	*nam;		// Array of names corresponding to Grps
	};

extern	DGRP	*dgrp[];	// Instantiated in calling app

struct CONTXT
	{
	char	typ;
	ulong	value;
	};


void	str2Context_DYNAG(const char *strContext, DYNAG *tbl);			// PREFERRED function

void	dgrp_dsp_nam(DGRP *g);
DGRP	*read_dgrp(char *fnam, int must_exist);
void	scrap_dgrp(DGRP **g);
void	scrap_dgrp10(void);
int		resolve_grp(DGRP *g);	// In DGRP1.CPP (uses '*Curv')
void	list_dgrp(DGRP *g);		// TESTING CALL
int		read_dgrp_cat(HDL f, char *dat, DGRP *g, char *rg_buff, int &ln, int &in_cat);


ulong	c2k(char *p,SY *s);
char	*k2c(SY *s, ulong v);

void	sy_move(int to, int from);
void	sy_move_timgrp(int tb_basis);
int		set_rollover(char *hm);

int		_cdecl cp_con(const char *a, const char *b);
int		_cdecl cp_reg(const char *a, const char *b);
int		_cdecl cp_pof(const char *a, const char *b);
int		_cdecl cp_bnm(const char *a, const char *b);
int		_cdecl cp_anm(const char *a, const char *b);
int		_cdecl cp_snm(const char *a, const char *b);

struct JNY_DET {ushort uniq, bd;};
ulong	unformat_key(int typ, char *s);

JNY_DET	*vb2jd(char *vbjd, JNY_DET *jd);

void	k2c_cal(char *w, long cal);
void	set_date_fmt(void);
extern	int	date_fmt;				// instantiated in dgrp1
ulong	fld_getl(char* &ad, int len);

void	dop_verify(void);
void	k2c_pad(char *w, SY *s);
void	alt2chr(char *s);

int		flgetln_alt2chr(char *str, char *org, HDL f);
int		getln_buff(HDL fp, char *buffer);
extern int can_include;

double	datfld_val(SY *s,char *ad);				// Returns value at &ad[s->koset] stored in s->len bytes

void	setup_cp(void);

int		_cdecl	cp_opk(const char *a, const char *b);
void	set_dynag_cp(void);

extern	char *op_key;		// List of key flds and offsets in opk elem (in dgrp.cpp)
int		set_week1(char *b2);
extern	int	week_as_date;
char	rotate_7bits(char flag, int shift);

void	change_sy_name(int typ, const char *newname);

const	char	*dop2daynam(char dop);
char	*get_dop_special(int digit);


class ALT2STRING
{
public:
ALT2STRING(const char *ip, int with_values);	// if with_values=True, each fld-id followed by txtval\tab
~ALT2STRING() {memtake(converted);};
char	*converted;
private:
};

int		get_defined_id(const char *str, int *len);

char	*get_subst(SY *s, char *w, int match);


struct	MAX_JNY_LEN {char *srv[4]; short item, jnylen;};

struct	SHTRIP;	// pre-declare...

class	Context_DYNAG {
public:
Context_DYNAG(const char *strContext) {str2Context_DYNAG(strContext,tbl=new DYNAG(sizeof(CONTXT)));};
~Context_DYNAG() {delete tbl;};
DYNAG	*tbl;						// table with one entry for each Field-ID in strContext
};


char	*tkt_name(ulong tpp);	// tpp is 'composite' typ<<16 | pax<<8 | pay


void	set_grp_max(DGRP *g, SY *s, int category_ct);

DGRP	*find_depot_tgf(void);
int		find_depnum(DYNAG *d, int target);
