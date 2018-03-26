extern	char	*cmpnam;
extern	long	runtime;		// this is instantiated in xsq.cpp, but other routines need to reference it

struct SQZHDR {short flag,mod_sz; long dte; char ff,spare;};	// 'flag tells us which compression method was used
struct SQZ {short sqz_sz; SQZHDR *sqh;};
#define C2AMAX 65
struct C2A
	{
	long	ctl, dttm_update, dttm_parse, fill[3];
	short	ver, ver_id_12345;
	short	unused_rollover, ct, bd[C2AMAX];
	};
struct DosC2A {long ctl,dttm_update; short ct,bd[C2AMAX];};

#define CURRENT_C2A_VERSION 1

struct CTL_MSZ {long mod; ushort sz;};

// 'match_bd' is non-zero ONLY in ...\preval\XREFA000.TKT (input to Schedule Matching Session)
// In this case there will only be ONE Subfile, because this archive subset contains all work for a single Activity Date 
// (it holds all discs containing match_bd, so it may include some/all data for other dates).
// 'Rematch' is also only set in this Subfile (if 'move to previous day' has been executed within Session)
struct ARC_CARGO
	{							// _TF Cargo associated with an archive Subfile (i.e. - CTL_BTR btree)
	short	match_bd;			// There will only be ONE Subfile if 'match_bd' is non-zero
	char	rematch;			// Set to 1 if ApplyUpdates affected Previous Day, else 0 (SJH - 29/03/04 - this is flakey)
	char	fillchar;
	RHDL	rhdl;
	char	fill[14];
	};

struct CTL_CARGO
	{							// _TF Cargo associated with a MOD_BTR btree
	long	f1,tf_ky;			// (i.e. - with a CTL_BTR entry for a disc)
	short	ccflag;				// Gives details about the disc's total data
	long	datsz;				// size, range of Activity Dates, _TF structure
	char	f3[4];				// used to interpret it, etc.
	long	f2;
	};

#define	TF_MXSZ		65000 			// Maximum size of _TF record (pre-050406 was 4000)
#define	TF_V4		0x80000000L		// tf::fmt bit set if using 2-byte xcodes
#define	TF_NEWCFG	0x40000000L		// New _TF config structure - actual size = 16 x (key>>20)
#define	TF_V4A		0x20000000L		// 060508 tf::fmt bit set if using 3-byte xcodes (leading 'kind' byte 0/1/2 = Tkt/Pax/Pay)

struct BD_DTTM {short bd; long dttm;};


struct	WFMT
	{
	char	twobyte,srvlen,ver,timtck,b3,alight,inssz,trip_id,trip_end;
	short	hdrsz;
	};


#ifdef documentation
The SQZHDR pointer is actually allocated enough space for one SQZHDR
followed by up to BufSZ bytes of compressed data - i.e. the address of
the SECOND element in the SQZHDR array is the compression buffer.

Besides clearing dirty buffers, flush() clears internal variable 'ctl',
which ensures that the next time put() is called it sees a new CtlDate.
This triggers the process of clearing any existing modules archived for
that date. Really, this is why get() has an optional parameter 'upd',
enabling XREF to re-save a new version of an individual Module file
WITHOUT clearing all the existing modules archived for the CtlDate.

get() can be called with mod_id=0 in which case it just returns YES/NO
according to whether any data has been stored for the CtlDate passed.

Note that we don't know which dates are archived in which database files!

The 'spare' rhdl in the archive database header is intended to give a
handle for expansion. It's always 0 in version 1, but later we could add
another record when the database is created, and use it for just about
anything that might crop up as useful information.
#endif


void	wfmt_setup(void);
int		cmpnam_sz(void);



class DYNTBL_int: public DYNTBL			// Derived class to handle table of 4-byte ints
{
public:
DYNTBL_int(void):DYNTBL(4,cp_ulong) {;};
int		get(int n) {return(*(int*)DYNAG::get(n));};
};


struct TKCFG {long ctl; char mask[8];};

struct FSC {short bd, id; long last_ctl;};	// FileSet Control Structure
