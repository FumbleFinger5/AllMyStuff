#define _CRT_SECURE_NO_WARNINGS 1

#include <stdarg.h>
#include <memory.h>
#include <string.h>


extern char logpath[];
extern	int	logopt;
#define LOGOPT_ROOT 1
#define LOGOPT_TIME 2
#define LOGOPT_EXIT 4

#define MEMLOG 1		// memgive	If set check buffer over/underrun and that all malloc's are freed

char *sjhtxt(char *buf, char *mode);	// read/write Buff to sjh.txt (mode=r/w)


// Append formatted text to sjh.log
void sjhlog(char *fmt,...);
#define sjhLog sjhlog	// Apps call sjhLog instead of sjhlog when it's a "real" call, not just testing stuff
#define sjhloG sjhlog	// TestCode call sjhloG instead when it's a commented out "debugging" call
void sjhlog_hex(char *p, int sz);

#define VBUFSZ 64000

#define Uint	unsigned int
#define ushort	unsigned short
#define uchar	unsigned char
#define ulong	unsigned long

//typedef int 	 (_cdecl *PFI)(...);	// pointer to function returning int
typedef	char 	 *HDL;	// abstract data type handle
typedef	long 	 RHDL;	// database record handle


#define MOVE8BYTES(a,b) (*(__int64*)(a)=*(__int64*)(b))
#define SAME8BYTES(a,b) (*(__int64*)(a)==*(__int64*)(b))

// Watch out for MOVE3BYTES - It evaluates 'a' twice!! (usually harmless, but keep an eye on it!)
#define MOVE4BYTES(a,b) (*(long*)(a)=*(long*)(b))
#define SAME4BYTES(a,b) (*(long*)(a)==*(long*)(b))
#define MOVE3BYTES(a,b) (*(long*)(a)=(*(long*)(b)&0xffffff)|(*(long*)(a)&0xff000000))
#define SAME3BYTES(a,b) (((*(long*)(a))&0xffffff)==((*(long*)(b))&0xffffff))
#define MOVE2BYTES(a,b) (*(short*)(a)=*(short*)(b))
#define SAME2BYTES(a,b) (*(short*)(a)==*(short*)(b))

// #define BIT8_AND(a,b) ((*(long*)(a)&*(long*)(b)) || (*(long*)(&((char*)(a))[4])&*(long*)(&((char*)(b))[4])))
#define BIT8_AND(a,b)	(  ((*(__int64*)(a)) & (*(__int64*)(b)))  !=0)	// TRUE if any bit is on in BOTH 8-byte flags
#define BIT8_ANDAND(a,b)	(*(__int64*)(a)) &= (*(__int64*)(b))		// Logical AND all 64 bits in 'b' into 'a'
#define BIT8_OR(a,b)	(*(__int64*)(a)) |= (*(__int64*)(b))			// Logical OR all 64 bits in 'b' into 'a'
#define BIT8_XOR(a,b)	(*(__int64*)(a)) ^= (*(__int64*)(b))			// Logical XOR all 64 bits in 'b' into 'a'
#define BIT8_ON(a)		(*(__int64*)(a) !=-0)							// TRUE if any bit is on in this 8-byte flag
#define BIT8_SET0(a)	(*(__int64*)(a)=0)								// Set all 64 bitflags in 8-byte area to zero

#define DTTM2DEP(dttm) ((char)((dttm)%60))


//	Some useful macros
//	General defines used almost everywhere
#define	ABS(x) 	((x)<0?-(x):(x))
#define	BKSP	8
#define	BIGLONG	0x7fffffff
#define	SPACE	' '

#define	CHR_HASH	35
#define	CHR_QT1	39

#define	COMMA	','
#define	SEMICOLON	';'
#define	CPMEOF  0x1A		// <ctl-z> byte for eof in text files
#define	CRET    '\r'		// 13
#define	DIV2(x)	(((unsigned)(x))>>1)
#define	EQ      0
#define	ESC		27
#define	GT      1
//#define	INRANGE(a,b,c)	(((b)<(a))?(a):(((b)<(c))?(b):(c)))
#define	ISALPHA(c)	((('a'<=(c))&&((c)<='z')) || (('A'<=(c))&&((c)<='Z')))
#define	ISHEX(c)	((('0'<=(c))&&((c)<='9')) || (('A'<=(c))&&((c)<='F')))
#define	ISDIGIT(d)	(('0'<=(d))&&((d)<='9'))
#define	LNFEED   '\n'		// 10
#define	LT       (-1)
#define	MAX(a,b) (((a)>(b))?(a):(b))
#define	MIN(a,b) (((a)<=(b))?(a):(b))
#define	MIN_BUFS 4				// Min number of database buffers reqd
#define	NO		 0

#define	NOTFND	 (-1)
#define	NOARCHIVE	 (-2)

//	NULL's for various types of objects
#define	NULLPTR	((void *)0)		// use to pass null pointer to funcs
#define	NULLHDL	((HDL)0)
#define	NULLRHDL	((RHDL)0)
#define	TAB			'\t'
#define	TOLOWER(c)	((('A' <= c) && (c <= 'Z')) ? ((c)+('a'-'A')):(c))
#define	TOUPPER(c)	((('a' <= c) && (c <= 'z')) ? ((c)-('a'-'A')):(c))
#define	ISLOWER(c) ((c)>='a' && (c)<='z')
#define	ISUPPER(c) ((c)>='A' && (c)<='Z')
#define	ISALPHANUMERIC(c) (ISDIGIT(c) || ISALPHA(c))

#define	SGN(n) (n?((n<0)?-1:1):0)

#define	YES		1

#define	DB_DIRTYPOS	38			// Offset of "dirty" flag byte database header
#define	DB_IS_CLEAN	'('		// Value in header while database is CLEAN
#define	DB_IS_DIRTY	'['		// Value in header while database is DIRTY
#define	PGSIZ		2048			// database page size
#define	FOOTERSIZ	24
#define	MAXRECSIZ	(PGSIZ-FOOTERSIZ-(3*sizeof(short)))		// Max size of (efficient) "single block" records
#define	MAXZRECSIZ	595800											// Even inefficient "multi-block" xrec's can't be bigger than this!

#define	DAY_SECS	(24L*60L*60L)
#define	ONE_DAY		86400L		// Number of seconds in 24 hours
#define	BD1980		3652		// (short) binary date value of 01/01/1980
//#define BIGDATE		24105		// Latest (short) date we accept = 31/12/35 (latest we COULD accept=24855 = 19/01/38)
#define	BIGDATE		21915		// Latest (short) date we accept = 01/01/2030 (latest we COULD accept=24855 = 19/01/38)
#define	BIGDATL		(BIGDATE*ONE_DAY)

#define	short_bd(dttm)	((short)((dttm)/ONE_DAY))
#define	long_date(bd)	((long)(bd)*ONE_DAY)
#define	short_hm(dttm)	((short)(((dttm)%ONE_DAY)/60))

class TIMER {
public:
TIMER	(void);
TIMER	(int delay_csecs, int initial_delay);
~TIMER ();
int tick(void);
void log(char *txt);
private:
int	iv, delay;
HDL	tmr;
};


int		bit_test(const char* m, int b);
void	bit8_set_one_bit(char *mask, int bitnum);
void	bit_set(char *m,int b);
void	bit_unset(char *m,int b);
int		bit_only1(char *m, int bit_ct);

char	*bd2vb(short bd, char *vb=0);
char	*hm_str(long dttm);			// hhmm
char	*hm_stri(int m);			// format hhmm string from binary minutes
int		a2hm(const char *p);		// fast & loose binary minutes from hhmm string


struct FILEINFO
	{
	ushort	attr;		// attributes of file : system, hidden, directory etc
	long	dttm;		// creation date and time of file
	__int64	size;		// size of file in bytes
	char	name[255];	// name of file (or directory)
	};


//	Seek mode parameters for B-tree Keys
#define	BK_LT		-2	// Seek key <  KEY
#define	BK_LE		-1	// Seek key <= KEY
#define	BK_EQ		0	// Seek key == KEY
#define	BK_GE		1	// Seek key >= KEY
#define	BK_GT		2	// Seek key >  KEY

//	Calendar Period types
#define	CAL_DAYS	0
#define	CAL_WKDAYS	1
#define	CAL_WKS		2
#define	CAL_MTHS	3
#define	CAL_QTRS	4
#define	CAL_YRS		5

//	Calendar error indicators
#define CE_YR		1
#define CE_MTH		2
#define CE_DAY		3
#define CE_HR		4
#define CE_MIN		5
#define CE_SEC		6


//	Data type codes for search functions and B-trees
#define	DT_STR	 	1
#define	DT_USHORT	2
#define	DT_LONG		3
#define	DT_BYTES	4
#define	DT_SHORT	6
#define	DT_ULONG 	7 
#define	DT_USR 		128		// User-defined comparator

//		DOS	file attributes
#define	FA_ORDINARY	0x00	// regular file
#define	FA_RDONLY	0x01	// Read only attribute
#define	FA_HIDDEN	0x02	// Hidden file
#define	FA_SYS		0x04	// System file
#define	FA_LABEL	0x08	// Volume label
#define	FA_DIR		0x10	// Directory
#define	FA_ARCH		0x20	// Archive


//	Function Return Codes
//#define	RC_SUCCESS	(0)			// Called function succeeded
//#define	RC_FAIL		(-1)		// Called function failed

#define SE_NOSHED		(-5)


//	Reserved Error Codes
#define ERR_ARC_NOCOPY	(1)		// DllCopyAll failed to write anything to o/p Ticket Archive
#define	ERR_ARC_MISSING	(3)		// ArchiveCopy source file doesn't exist
#define ERR_ARC_DUPLICATES (99)	// ArchiveCopy duplicate discs warning
#define SE_FLDID		(92)	// Unknown Field-ID

#define	SE_USER_CANCEL	(-200)	// Processing aborted by user or untrapped error

//	-257 to -299	Miscellaneous
#define	SE_OUTOFHEAP	(-258)	// No memory for memgive()
#define	SE_BADALLOC		(-261)	// Attempt to memgive() 0 bytes
#define	SE_MEMBOUNDS	(-262)	// Write before/after memory block
#define	SE_MEMCHK		(-263)	// MEMCHK tables are corrupt!
#define	SE_MEMDLR		(-264)	// Dynamic linked list pointer error
#define	SE_MEMBAD		(-266)	// Memory buffer under/overwrite
#define	SE_MEMPTR		(-267)	// Attempt to release unallocated pointer
#define SE_BUFF64K		(-268)	// 64 Kb api buffer overwrite (pre 05/12/06 was -581)
#define SE_MEMTBL		(-269)	// Error merging memory tables
#define SE_DATE			(-280)	// Date/Day/TermType error

//	-300 to -320	OS high-level filing system
#define	SE_ERR_PATH		(-300)	// Error accessing directory
#define	SE_ERR_FILE		(-301)	// Error accessing file
#define	SE_ARC_EXISTS	(-302)	// File exists that logically should not (pre 05/12/06 was -201)
#define	SE_ARC_MISSING	(-303)	// File missing that logically must exist (pre 05/12/06 was -202)
#define	SE_RENAME		(-304)	// OS error renaming file (pre 05/12/06 was -202, -1)
#define SE_MOD_UPD		(-305)	// Error applying updates from file into Ticket Archive

//	-321 to -349	OS low-level IO system
#define SE_BADWRITE		(-321)	// write error
#define SE_BADREAD		(-322)	// read error
#define SE_BADSEEK		(-323)
#define SE_SJHLOG		(-324)
#define SE_RD_ONLY		(-325)	// Tried to write to file opened as ReadOnly

#define SE_TMPCREATE	(-330)	// Couldn't open TMP_DB

// -350 to -448	Database system
#define SE_NXTKEY		(-333)	// bad bkynxtkey()
#define SE_PRVKEY		(-334)	// bad bkyprvkey()
#define SE_BADPGID		(-385)	// bad page id in record handle
#define SE_BADCELLID	(-386)	// bad cell id in record handle
#define SE_NOFREEBUFS	(-387)	// no free buffers in buffer pool
#define SE_DBPGBAD		(-388)	// smashed database page
#define SE_DBOVRFLOW	(-389)	// database file too big (>64 megabytes)
#define SE_RECTOOBIG	(-390)	// Size of stored record bigger than page
#define SE_REC_WRITE	(-391)	// Error writing database record
#define SE_NULLHDL		(-392)	// Attempt to activate 'db' handle 0
#define	SE_NOTACTIVE	(-393)	// Database system not started
#define SE_DBCORRUPT	(-398)	// Database record is obviously invalid (unexpected size, etc)
#define SE_DBSAFE		(-399)	// [DB_SAFE] failure


//	-449 to -512	B-tree Indexes
#define SE_BADKEYTYP	(-449)	// bad key type in compare
#define SE_BADDELKEY	(-450)	// error during key deletion
#define SE_BADBROOT		(-451)	// Root of tree appears bad
#define SE_BADPARENT	(-452)	// Error rebuilding parent after Bkydel()

#define SE_TKTNDX		(-501)	// Corrupt Ticket Archive Index (pre 05/12/06 was -777)
#define SE_SQL_GET		(-502)	// Error getting recordset from SQL database (pre 05/12/06 was -101)
#define SE_C2A_GET		(-503)	// Error getting Ticket Archive disc details (pre 05/12/06 was -101)
#define SE_NDX_GET		(-504)	// Error reading Ticket Archive Index (pre 05/12/06 was -1002, -1003)
#define SE_SQL_PUT		(-505)	// Error adding record to SQL database (pre 05/12/06 was -101)
#define SE_MOD_GET		(-506)	// Error reading ModFile from Ticket Archive (pre 05/12/06 was 108)
#define SE_NDX_PUT		(-508)	// DllCopyAll failed to Index Ticket Archive (pre 05/12/06 was 2)
#define SE_SQL_EXEC		(-509)	// Error executing SQL statement

#define	STR_LJUST 	0			//ustification types for strjust()
#define	STR_RJUST 	1
#define	STR_CENTER 	2


void	memtake(const void *);

extern int dbsafe_catch;

extern	char *caller_name;

												// If non-NULL, delete & zeroise...
#define Scrap(a) scrap((void**)(&(a)))			//		ptr-> allocated memory
#define SCRAP(a) {if (a) {delete (a); (a)=0;}}	//		ptr-> class object

void swap_data(void *a, void *b, int sz);		// swap 'sz' bytes of data between a & b

//#define mempp memmove
//#define mempb memset
//#define stracpy strcpy
//#define stralen strlen
//#define memcmpb memcmp			// int memcmpb(const void *a, const void *b, ushort bytes_to_compare);

// get/put _binary124 use Intel "backwords" native binary storage mode
ulong get_binary124(const void *addr, int len, int subscript);
void put_binary124(void *addr, ulong value, int len, int sub);

#define stracat strcat
char *stradup(const char *str);						// (makes sure PF memgive() gets used for this)
//#define stradup strdup		// ##### NO NO NO!!!! - MSC uses a different memory allocation system to memgive()!!! ####
#define strupper(s) _strupr_s(s,strlen(s)+1)
#define strancmp strncmp
//#define stracmp strcmp
//#define strancpy strncpy	// NO NO NO - Msc version doesn't write terminating nullbyte if source exceeds o/p buffsize
char	*strancpy(char *dst, const char *src, int bufsz);
char	*stracpy2(char *dst, const char *src, char up2);
char	*strcpy_trim(char *op, const char *ip, int maxlen);	// maxlen is how much of "ip" to look at, not how much to op!

//#define sylongjmp(x) throw x

#define FNAMSIZ 256				// Max fully-qualified path is 255 chars + nullbyte
#define TEXT_LINE_SZ 256		// size of buffer into which text files are read (incl 1 null byte)

#define QRY_LINE_SZ 512			// size of buffer into which text files are read (incl 1 null byte)


typedef int	(_cdecl *PFIi)(int);
typedef int	(_cdecl *PFI_i)(int*);
typedef int	(_cdecl *PFI_v)(const void*);
typedef int	(_cdecl *PFI_v_v)(const void*, const void*);
typedef void(_cdecl *PFV_v_c)(const void*, char*);
typedef int	(_cdecl *PFIv)(void);
typedef HDL	(_cdecl *PFHi_v)(int,void*);
typedef void(_cdecl *PFVh)(HDL);
typedef void(_cdecl *PFVv)(void);
typedef void(_cdecl *PFV_v)(void*);
typedef int	(_cdecl *PFIi_v)(int,void*);
typedef HDL	(_cdecl *PFIh_v)(HDL,void*);

int		cp_bytes(const void *p1, const void *p2);
int		cp_strn_ks0(const void *p1, const void *p2);
int		cp_mem1(const void* p1, const void* p2);
int		cp_mem2(const void* p1, const void* p2);
int		cp_mem3(const void* p1, const void* p2);
int		cp_mem4(const void* p1, const void* p2);
int		cp_mem6(const void* p1, const void* p2);
int		cp_mem8(const void* p1, const void* p2);
int		cp_short(const void *p1, const void *p2);
int _cdecl cp_short2(const void *a, const void *b);
int		cp_ushort(const void *p1, const void *p2);		// the values as parameters

int		cp_long(const void *p1, const void *p2);
int		cp_ulong(const void *p1, const void *p2);
int _cdecl cp_ulong2(const void *a, const void *b);
int _cdecl cp_ulong4(const void *a, const void *b);


int		cp_short_v(short a, short b);					// These 4 take the ACTUAL
int		cp_ushort_v(ushort a, ushort b);				// values as parameters
int		cp_long_v(long a, long b);
int		cp_ulong_v(ulong a, ulong b);

ushort	r2i(const char *str);			// ASM routines for keyfld_c2i()
ulong	r2l(const char *str);			//					keyfld_c2l()
ulong	r2long(char *ad, int len);
short	*r2i2(short *i2, const char *r2);	// Convert TWO reverse shorts. Echo back passed ptr_short

char	*i2r(ushort v);					//					keyfld_i2c()
char	*l2r(ulong v);					//					keyfld_l2c()
ushort	a2i(const char *str, int len);
ulong	a2l(const char *str, int len);
int		a2l_signed(const char *str, int len);
extern	int a2err, a2err_char;			// Error on last call to a2l() (0=OK, 1=Fail, 2=EmptyString. If 1, a2err_char is the non-digit we found

char *bit2c(int flag, int numbits);

int	bkyadd(HDL btr,RHDL rhdl,void *key);					// YES/NO = OK/Fail
int	bkydel(HDL btr,RHDL *rhdl,void *key);					// delete key only (YES/NO = OK/Fail)
void	bkydel_rec(HDL btr, void *key);							// delete key AND rhdl
void	bkydel_all(HDL h_btr, int del_rhdl);					// del ALL keys and maybe recs
int	bkyupd(HDL btr,RHDL *rhdl,void *key);					// YES/NO = OK/Fail
int	bkyscn_all(HDL btr,RHDL *rhdl,void *key, int *again);
int	bkysrch(HDL btr, int mode,RHDL *rhdl,void *key);		// YES/NO = OK/Fail
void	*btrcargo(HDL btr,void *cargo);			// get/update 10-byte btr cargo
void	btrclose(HDL btr);
long	btrnkeys(HDL btr);
void	btrrls(HDL btr);
void	btr_set_cmppart(HDL btr, PFI_v_v func);	// (db internal)
RHDL	btrist(HDL btr, int keytyp, int keysiz);
HDL	btropen(HDL btr,RHDL rhdl_btr);

int	calerr(void);
char	*calfmt(char *str,char *ctl,long cal);
long	caljoin(int yr,int mo,int dy,int hr,int mn,int sc);
long	calnow(void);
void	calsplit(long cal,short *yr,short *mo,short *dy,short *hr,short *mn,short *sc);
char	*cal2VbDate(long cal);
char	*cal2Vb1Date(long cal, bool start);
void	cal_check_date_range(short bd0, short bd1);

char	*dbfnam(HDL db);				// Fully-qualified pathname of *.DBF file
void	dbckpt(HDL db);
void	dbsafeclose(HDL db);			// Close database AND unset the db.hdr 'safe' flag if it was set to say 'dirty'
RHDL	dbgetanchor(HDL db);
//short	dbisrhdl(HDL db, RHDL rhdl);	// (db internal)
int	dbist(const char *nam);			// YES/NO = OK/Fail
int	dbsetlock(int on);				// Lock db system against ALL file updates
HDL	dbopen(const char *nam);
ushort	dbpgct(HDL db);
RHDL	dbsetanchor(HDL db,RHDL rhdl);	// note - returns RHDL, not void
void	db_set_temp(HDL db);			// Deletes on close
void	dbstart(int nbufs);
void	dbstop(void);
int		dbreadonly(HDL db);

int		drattrget(const char*,short*);
int		drattrset(char*,int);
int		drchdrv(int);

char	*drfulldir(char*, const char*);		// expanded dirspec (ends in '\')
char	*drfullpath(char*, const char*);	// expanded filspec
int		drinfo(const char *path, FILEINFO *fi);	// YES/NO = OK/Fail
int		drisdir(const char *directory_path);	// YES if it's a directory and it exists, else NO
void	setcwd(const char *path);
char	*drsplitpath(char *dir_only, const char *fullpath);
int		drunlink(const char *filename);		// YES/NO = OK/Failed to Delete
//void	drrmdir(const char *filename);								// _rmdir
int		drrename(const char *oldfilename,const char *newfilename);	// EQV rename(), but returns OK/NO instead of errorcode


struct	DRSCN;	// pre-declare so compiler knows there will be such a structure
DRSCN	*drscnist(const char*);
int		drscnnxt(DRSCN *scn,FILEINFO*);			// YES/NO = OK/Fail
void	drscnrls(DRSCN *scn);

short	memidxw(short val,const short *ary,short tsize);	// find 'val' in (unsorted) table 'ary'
/* void	memtakering (int off, HDL ring); */
//void	prtfmt(char *fmt,...);
RHDL	recadd(HDL db, const void *rec,short sz);
void	recdel(HDL,RHDL);
int	recget(HDL db,RHDL rhdl,void *rec, int mxsz);
void	recupd(HDL db,RHDL rhdl,const void *rec, int sz);

void	set_maxc(char *hi, int v);				// Set '*hi' to 'v' if v is bigger
void	set_maxc_str(char *hi, char *str);		// (as above, but 'v' is strlen(str)

char	*_strfmt(char *str,const char *fmt,va_list);
void	strendfmt(char *str, const char *fmt,...);
char	*_strnfmt(char*,int, const char*, va_list);
char	*strend(const char *str);
char	*strfmt(char *str,const char *fmt,...);
char	*strdel(char *str, int ct);				// delete 'ct' chars from front of 'str'
char	*strfill(char *str, int ct, int chr);	// (sets str[ct]=0)

char	*strins(char *str, const char *ins);	// add 'ins' to front of 'str'
char	*strinsc(char *str, int ins);			// add char 'ins' to front of 'str'
int	stridxc(char,const char*);
int	stridxs(const char *substr, const char *str);	/* substr offset in str, or NOTFND */
int	strinspn(const char *any_of_these, const char *in_this_string);
char	*strjust(char* str, int width, int type, int fillchr);
char	*strnfmt(char*, int len, const char*,...);
char	*strrjust(char* str, int width);
int	strtoken(char*,char*,char*);
char	*strtrim(char*);
//char	*strupper(char*);
const char	*str_field(const char *str, int n);

int	syenvstr(char*,char*,int);
long	syticget(void);
void	sytimget(int *hr, int *mm, int *sec, int *csec);
HDL	tmrist(long);
long	tmrelapsed(HDL);
void	tmrreset(HDL,long);
void	tmrrls(HDL);

ulong	hex2l(const char *s, int len);					// i/p = charstring of hexdigits (1 per byte)
long	x2l(const char *base, short offset, short len);	// i/p = BCD hex nibbles (2 per byte)
void	l2x(char *base, short offset, short len, long val);
void	bd2x(char *hex_addr, int bd);
void	hm2x(char *hex_addr, int hm);
char	*x2c(char *dst, const char *src, int nibble_ct);
ulong	h2l(char *s, int nibble_ct);			// return unsigned binary value from a string of hex nibbles

int	zrecsizof(HDL db,RHDL rhdl);
int	zrecget(HDL db, RHDL rhdl, void *data, int sz);
RHDL	zrecadd(HDL db, const void *data, int sz);
RHDL	zrecupd(HDL db, RHDL rhdl, const void *data, int sz);
void*	zrecmem(HDL db, RHDL rhdl, int *sz);

void	*memadup(const void*,Uint);
void	*memgive(Uint);
void	*memrealloc(void*,Uint);

void	scrap(void **pointer);
void	memchk(char *txt);
int	memtakeall(void);		// Release all blocks allocated by memgive/memrealloc

HDL	flopen(const char*,const char*);
HDL	flopen_trap(const char*,const char*);
void	flclose(HDL);
int	flcloseall(void);
void	flckpt(HDL);
int	fleof(HDL);
ushort	flget(void*,ushort,HDL);
int	flgetln(char*,int,HDL);
char	*flnam(HDL h_fl);
ushort	flgetat(void *buffer,ushort bytes,long pos,HDL fl);
void	flput(const void*,int,HDL);
void	flputc(int,HDL);
void	flputat(const void *buffer, int bytes, long pos, HDL fl);
void	flputs(const void *s, HDL f);
void	flputln(const void* line, HDL fl);
void	flsafe(HDL);					// set hdr flagbyte to say "database is safe"
long	flseek(HDL,long pos,int mode);	// mode 0/1/2 = from BEG/CUR/END
void	flnam_env(char *p, const char *fn, const char *envstr);

int	digits(double v);			// No. of digits needed to store 'v'

void	vb_append(char *vbuf, const char*fmt,...);
char	*vb_item(const char *str, int n, char *copy_to=0);

char	*vb_field(const char *rec, int n);
void	vb_bd2(short *bd, char *str, int sub); // Decode two dates in vb_fields sub & sub+1 into bd[]

ulong	a2lx100(const char *s);	// Conversion function
char	*lx1002c(ulong v);
char	*ctl2c(long ctl);			// Conversion function
long	c2ctl(const char *str);
char	*ctlmod2c(long ctl, long mod);

int	force_into_range(int value, int min, int max);

short	rnd(short lo,short hi);


int		__stdcall DllGetLastError(char *ErrTxtBuffer);			// make visible to all app code...
void	SetErrorText(const char *fmt,...);					// Set text to be gotten by next DllGetLastError() call
void	SetErrorText_if_blank(const char *fmt,...);			// Set text only if currently blank
int	RetErrorText(int errcode, const char *fmt,...);		// As SetErrorText(), but returns 'errcode' as a convenience
void	m_finish(const char *fmt,...);
void	sjhlog_file_error(const char *fnam, const char *txt, ulong dont_log_this_value=987654321);


int	highest_code_in_string(char *p);
int	time_diff(int a, int b);

int	in_table(int *p, const void *ky, void *tbl, int ct, int sz, PFI_v_v cp);
int	to_table(void *ky, void *tbl, int *ct, int sz,	PFI_v_v cp);
int	to_table_s(void *ky, void *tbl, short *ct, int sz,	PFI_v_v cp);

class TAG {			// A class to dynamically allocate space for any number
public:					// of variable (elemsz=0) or fixedlen (elemsz>0) strings
TAG	();	// and retrieve any item by number as requested
TAG (int first);
~TAG	();
int leaks(void);
private:
int	id;
};
int leak_tracker(int start);

class DYNAG: public TAG
{										// Class to dynamically allocate space for any number
public:								// of variable-length strings (_sz=0) or fixedlen items (_sz>0)
DYNAG	(int _sz, int _ct=0);	// and retrieve any item by number as requested
DYNAG (DYNAG *copyfrom);
~DYNAG	();
void	*put(const void *item, int n=NOTFND);	// Add item to table & return addr
void	*puti(int i);							// (just a little wrapper because we often store 1-2-4 byte int's in tables)
void	*get(int n);					// Retrieve address of item 'n'
void	del(int n);						// Delete item 'n' (use del(n)+put(n) to update)
int	ct;								// No. of items in Category (s/b read-only!)
int	sz;
int	in(const void *item);			// Return subscript of 'item' if in table, else NOTFND
int	in_or_add(const void *k);
void	*cargo(void *data, int sz=0);	// general-purpose storage area within the class object (default=NULL=unallocated)
protected:
int	find_str(int *p, const void *k);
char	*a;
int	len;
private:
void	init(int _sz, int _ct);
DYNAG	*slave;							// Offset of each entry in main table if vari-length (elemsz=0)
int	eot,cargo_sz;
void	*_cargo;						// (app code can't directly access the private cargo pointer)
};

// **** Note - 'put(0)' squeezes table to eliminate alloc'ed slack at end


class DYNTBL: public DYNAG		// Derived DYNAG class with sorted table
{								// put() doesn't add items already present
public:							// Contructor takes comparator as well as size
DYNTBL(int _sz, PFI_v_v _cp, int _ct=0);
void	*put(const void *k);
void	*puti(int i);			// (just a little wrapper because we often store 1-2-4 byte int's in tables)
void	del(int n);				// Delete item 'n' (use del(n)+put(n) to update)
int	in(const void *k);
int	in_or_add(const void *k);
int	in_GE(const void *k);	// EQV in() except it returns subscript as per BK_GE (if there is such a key in table)
void	*find(const void *k);
void	merge(DYNTBL *add);
int		set_cp(PFI_v_v _cp);
private:
PFI_v_v	cp;
};

short	*geti2(short *i2, DYNAG *d);	// Copy FIRST & LAST (short) values from 'd' to 2-element array i2
int		return_up_to_64K(char *vb, DYNAG *tbl, void (*fun)(const void*,char*));
void	fmt_str(const void *str, char *vb);
void	fmt_bd(const short *bd, char *s);

// This trivial object clears ErrorText when instantiated, and remembers address of 'err'
// If a db error occurs, the destructor sets ErrorText so we know which file it was
class DBERROR
{
public:
DBERROR(int *_err);
~DBERROR();
private:
int	*err;
};


class QSTR {						// Class to make a quoted string (or can use other delimiters)
public:
QSTR(char *ip, int qt=34);
~QSTR();
char *str;
};

int short_overlap(short *a, short *b);	// 2 lo-hi ranges (if either is {0,0} they definitely overlap)

bool in_rng(short val, short*rng);



class TMP_DB: public TAG
{
public:
TMP_DB(void);
~TMP_DB();
HDL	db;
};

class TMP_BTR
{
public:
TMP_BTR(int type, int size, PFI_v_v cmppart);
int add(RHDL r, void *k);
int srch(int m,RHDL *r,void *k);
HDL		btr;
//private:
TMP_DB	d;
};

struct _TMR
	{
	long	secs;
	int		ticks;
	};


class BitMap
{
public:
BitMap(int _ct);
~BitMap();
void	clear_all(void);
void	set(int b);
void	unset(int b);
int		is_on(int b);
int		first(int on_wanted);	// return subscript of first bit that's ON (or first that's OFF if on_wanted=False)
int		Bit_only1(void);
private:
int		ct;
char	*bm;
};

