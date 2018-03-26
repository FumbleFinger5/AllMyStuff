#ifdef docum
This class retrieves Service, Running Order, and Stage data from ssn.dbf
(a variant of the class is used by SSN_LOAD to populate the database).

Set_srv() can be called to control which SrvNo is subsequently accessed by
stg2XXX() functions, or a SrvNo can be explicitly passed to them each time.

If the site doesn't have GlobalLocationCodes then we invent unique values
with the l6384-bit set. All stage-related details are accessed through the
GLC value

Public variable 'dep' is set by app to control what may be returned:-
0-59	only if there is a Service definition applicable to that depot.
60		only if there is a SINGLE definition for the Service.
61		simply returns the first definition found (if any).
#endif

struct	SSN_DET
	{
	char	name[32];
	short	stg[1];
	};

struct	SSNU {char srv[4], dep;};

struct	SSN_KEY {char srv[4];  DATE_DEP_SCOPE s;};

struct	SSNREC {int ID; SSN_KEY k; char name[32];};

struct	SSN_GLC
	{
	char	name[32], zone, f3[3], fill[8];
	long	grdref;
	};

struct SRVi {char srv[4]; short i;};		// i may be jny or stg, depending on context

int	_cdecl cp_srv_dsp(const char *a, const char *b);	// Srv2dsp (leading spaces) compare
int _cdecl cp_srvi(const SRVi *a, const SRVi *b);			// Compare Srv+ShortInt structure (int = Jny or Stg)
int _cdecl cp_ssnrec(SSNREC *a, SSNREC *b);

struct SSNSTG {short seq, stg; char name[32];};

char	*ssrt1(const char *c, char *s=NULL, int sq=1);

struct STGREC {short stg, ordinal; int glcID, ServiceID; char name[32];};

class SqlDB;
class DLR_CACHE;

class SSN {
public:
SSN(void);				// Constructor used by app progs
~SSN();
void	set_dep(int dep, int bd, const char *srv=NULL);	// Set codes & flush any existing def'n
int		set_srv(const char *srv);	// Return StgCt if OK, else 0
short	stg2idx(short stg,char *srv=0);	// returns ordinal position of 'stg' in Running Order, or NOTFND
int		stg2glc(short stg,char *srv=0);	// return GlobalLocationCode of 'stg'
short	glc2stg(int glc,char *sv, char depot);
char	*stg2nam(short stg,char *name=0);	// Copy StgNam to str & Echo
char	*srvnam_or_default(void);
DYNTBL	*get_srvnam_all(DYNTBL *ign);
int	ct;
SSN_DET	*sn;	// 2 x 'ct' short tables: StgNo, GlobalLocationCode

void	add_defaults(SSNU *su, int ct, int bd);

char	*srvnam(char *srv, short bd1, short bd2, char *mask);	// return the one & only name, or "(Multiple ServiceNames)"
DYNTBL	*GetSSNstage(SSNREC *sr);

protected:
SSN_GLC	g;
int		dep,keychg,bd;
SSN_KEY	sv;

private:
void	GetSSN(DYNTBL **tbl);
void	GetSSN1(DYNTBL **tbl, const char *srv);
int		uPd(SSNREC *s, SSNREC *prv);	// Only for SQM kludge to stop PM seeing un-named services
int		read_sn(int ID);
SqlDB	*a;
int		ErgGlc;
DYNAG	*gnam;			// table of all stagenames referenced
DLR_CACHE *dlr;
};


