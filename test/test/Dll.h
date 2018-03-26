//////////////////////////////////////////////////////////////////
// Declarations for COMMON functions used by VB / C++ INTERFACE  //
//////////////////////////////////////////////////////////////////
// Functions return 0 on success, error code on failure
#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>

// Structure defining the format of dates passed thru the i/f
//struct VB_STR_DATE {char dd[2], slash1, mm[2], slash2, cccc[4], sep;};

#define VBUFSZ 64000

// BitFlags that can be OR'd into 'flag' as passed to dllinit()
#define XA_RENUM	1		// Renumber Depots according to LOCATION group file
#define XA_CLEAN	2		// Check for invalid data and auto-repair where possible
#define XA_INDEX	4		// Maintain indexes on o/p archive
#define XA_NODUPS	8		// Delete (pref. from destination) fully duplicated discs / modules (not trips yet) before doing the copy
#define XA_PARSE	16		// Rebuild Activity Date list in C2A for each disc processed
//#define	XA_DUPDSK	32		// Clear any pre-existing copies of incoming discs in ANY archive subfiles
#define XA_DEPCOPY	64		// Use DEPCOPY.tgf to filter/renumber depots
#define XA_EXTRACT	128		// Create LA Extract archive
#define XA_DUPCHK	256		// Don't copy modules already in destination archive (which MUST be indexed)
#define XA_ORGCTL	512		// Use F6Type:35 (OrgCtl) values from source to decide whether to add/replace discs in destination


// Register Status callback (app's function to display progress so far)
int __stdcall DllRegisterStatusFn(void (__stdcall *funcptr)(int done, int total));

// Save address of callback function defined in calling application
int __stdcall DllRegisterCancelFn(int (__stdcall *funcptr)(void));

// Release any resources before DLL object is released
int __stdcall DllClosedown(void);


// Get text description of the error signalled by most
// recent DLL function returning a non-zero error code
int __stdcall DllGetLastError(LPSTR lpRec);

// Get 'version' text string descriptor for current DLL
int __stdcall DllGetVersion(LPSTR lpRec);

// Return count of supported Field-ID's copied to 'str'
int __stdcall DllGetFieldList(char *str);

// Populate 'vbuf' with as many MODLST print lines as will fit into 64 Kb and return NO error
// Release internal table if there aren't any more lines, or if vbuf is passed as NULL (quit early)
int __stdcall DllWaybillGetLinesEx(char *strContext, char *vbuf);	// New 270905 replaces DllWayBillX...X


// Copy tab-separated 'control' info for field 'typ' into 'buffer'
// Return 92 if 'typ' isn't a valid Field-ID, otherwise 0 (No Error)
// Fields returned in 'buffer' are positionally-defined:
//	NAME		(text)
//	DATATYPE	D:Date (as ddmmyy)
//				T:Time (as hhmm)
//				C:Character (alphanumeric)
//				H:Hex digits (0-9, A-F only)
//				F:Floating Point (Decimals will be 1 or 2)
//				I:Integer (scalar - i.e. countable)
//				N:Numeric (non-scalar)
//	DECIMALS	0-2 (non-zero values only meaningful for DataType:F)
//	JUSTIFY		L:Left, R:Right, C:Centre
//	LENGTH		Maximum length of user-input value
//	SEQNO		Y:Extra 'SeqNo' column returned by dllGetnextRecord(), otherwise N
int __stdcall DllGetFieldInfo(int typ, LPSTR buffer);
int __stdcall DllGetFieldInfoEx(int typ, LPSTR buffer);	// As above, but adds Mnemonic



// Returns 1,2,3,... for Sun, Mon, Tue,...
int __stdcall DllFirstDayOfWeek(void);

int __stdcall DllGetDayDateScope(char *DayDateScope, char *ddmmyy);



// Populates "value" with text setting for [name] from TI.INI
// Checks any/all TI.INI files in folders specified by DllInit(search_path,...) - default (cwd);(main);(Queries)
// See TI_INI.DOC for SettingNames
// See XLIB.H for bitflag settings TIV_xxx
int __stdcall DllGetSetting(LPSTR name, LPSTR value, int bitflag);

int __stdcall DllGetArchiveCargo(char *strCargo, int subfile);		// UNSAFE! - passed buffer might not be big enough!
int __stdcall DllGetArchiveCargoEx(char *strCargo, int subfile);	// (this version uses 'repeat call' convention)
int __stdcall DllPutArchiveCargo(char *strCargo, int subfile);		// Supports len(strCargo) up to about 650Mb

int __stdcall DllVerifyUpdate(LPSTR upd);		// Check updates to module files
int __stdcall DllGetStageList(char *key, char *vbuf);

#define SE_NODATA		(1)
#define SE_BAD_STATE	(2)
#define SE_ERROR_LOG	(4)



char *Xsjhtxt(char *buf, char *mode);
int Xlist(char *p, char *txt, int loop);
int Xlistrevpax(int typ, char *v, int iso_date, char *rev, char *pax);
// Write formatted text to screen and Echo to text log file WXP.LOG
void Xecho(const char *fmt,...);

void Xlist_fields(char *p, char *txt);	// list \t-terminated fields in a \n-terminated record

int Xlist_and_pick(char *p, char *txt);	// xlist fields & pick one
