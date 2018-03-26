#include "pdefs.h"
#include "dlllog.h"
#include "stdlib.h"
#include "process.h"

// Metro have a few archived ModFiles that expand to be bigger than the recorded size.
// This fix reads the module if there are <= 16 extra bytes, otherwise gracefully refuses
// 17/04/07 - all relevant code for this logic is now conditionally compiled using #ifdef
//#define METROFIX YES


struct DATE_DEP_SCOPE {short intro; char mask[8];};
int _cdecl cp_date_dep_scope(DATE_DEP_SCOPE *a, DATE_DEP_SCOPE *b);
int _cdecl cp_mask(const char *a, const char *b);

#define LINECT 60
#define QP_MAX_LINE_LEN 128


struct	MODFIL_KEY
	{
	long	mod_id, ctl;
	};

void	report_status(int done_so_far, int total);
int		user_cancel(int dont_crash);


extern	char *buff;	// instantiated in xcom1.cpp
extern	char *bp;	// instantiated in xcom1.cpp

extern	const char *sy_daynam[];
extern	const char *sy_mthnam[];

extern int	dop_adj;	// (instantiated in dgrp1.cpp, accessible to appcode)

// parameter 'b' for Dop macros is normally a (short) binary date
#define	DOP(b) (((b)+4)%7)	// 0,  1,  2,  3,  4,  5,  6
#define	DOPB(b) (1<<DOP(b))	// 1,  2,  4,  8,  16, 32, 64
							// Sun,Mon,Tue,Wed,Thu,Fri,Sat
// DOP and DOPB are used internally, and CAN'T be altered
//					(DRIVER.n must always be 0=Mon, 1=Tue, etc.)

#define	DOP06(b) (((b)+11-dop_adj)%7)
// DoP06 Returns 0=Sun, 1=Mon, 2=Tue, etc., if Dop_adj=0.
//			Set Dop_adj to 1,2,... for Weekstart=Mon,Tue.,,, in DSEL, etc.
//			(This macro would then return Sun=6,5,... - which IS what the apps want!)
//
//			Apps use Sy_daynam[(i+Dop_adj)%7] if looping i=0-6 for names

#define	DOPFLAG(b) (1<<DOP06(b))			// 1,2,4... Mon,Tue,Wed...

struct	DOPTERM {char dop, term;};

int		dopb2dop06(int dop);		// converts DOPB bitflag for ONE day to that day's DOP equivalent 0-6
int		daynam2num(char *daynam);	// return DOP 0-6 value for DayName (that must be at least 2 chars long)
void	load_daysop(int do_it);


#define offsetof(t,i)	((size_t)((char *)&((t *)0)->i - (char *)0))
// Given structure type 'STRU', containing an element 'elm', then
// offsetof(STRU,elm) returns the offset of 'elm' within 'STRU'

// These macros compare just the 60 bits we use in depot masks
#define BIT60_AND(a,b) ((*(long*)(a) & *(long*)(b)) || (((*(long*)(&((char*)(a))[4]))&0xFFFFFFF) & ((*(long*)(&((char*)(b))[4]))&0xFFFFFFF)))
#define BIT60_EQ(a,b) ((*(long*)(a) == *(long*)(b)) && (((*(long*)(&((char*)(a))[4]))&0xFFFFFFF) == ((*(long*)(&((char*)(b))[4]))&0xFFFFFFF)))

#define CON_IDLEN	24	// Max Length of ContractNumber (excl. terminating EOS null-byte)
#define NP_IDLEN	16	// Max Length of N&P RegNo (excl. terminating EOS null-byte)
#define POF_IDLEN	24	// Max Length of PointOfFailure text

#define	IX_RUNALT		((char)1)	// AlightStage position in Running Order
#define	IX_RUNBRD		((char)2)	// BoardStage position in Running Order
#define	IX_ALTNUM		((char)65)	// 'A'	Alight Stage Number (0-9999)
#define	IX_BRDNUM		((char)66)	// 'B'	Board Stage Number (0-9999)
#define	IX_CLASS		((char)67)	// 'C'	Wayfarer TktClass (1 hex digit 0-E)
#define	IX_DATE			((char)68)	// 'D'	Trip Date
#define	IX_ETM			((char)69)	// 'E'	ETM Number (1-6 alphanumeric)
#define	IX_TRAVEL		((char)70)	// 'F'	Number of Stages Travelled
#define	IX_TRNTYP		((char)72)	// 'H'	Wayfarer 'Transaction Type' 1-7
#define	IX_TIME			((char)73)	// 'I'	Normally 'Trip Depart Time'
#define	IX_JOURNEY		((char)74)	// 'J'	Journey Number 0-9999
#define	IX_TKTTIM		((char)75)	// 'K'	Ticket Issue Time
#define	IX_DOT			((char)76)	// 'L'	Direction of Travel 0/1/2 Out/In/Unknown
#define	IX_MODULE		((char)77)	// 'M'	Module Number
#define	IX_WAYTKTNAM	((char)78)	// 'N'	Wayfarer Ticket Type Name
#define	IX_DEPNUM		((char)79)	// 'O'	Depot Number 0-59
#define	IX_PRICE		((char)80)	// 'P'	Ticket Price
#define	IX_JNYREV		((char)81)	// 'Q'	Total Trip Revenue
#define	IX_DRIVER		((char)82)	// 'R'	Driver Number (1-6 digits)
#define	IX_SRV			((char)83)	// 'S'	Service Number (1-4 alphanumeric)
#define	IX_WAYTYP		((char)84)	// 'T'	Wayfarer-style Ticket Type 0x00-0xED
#define	IX_DUTY			((char)85)	// 'U'	Crew Duty (1-6 digits)
#define	IX_VEHICLE		((char)86)	// 'V'	Vehicle Number (1-6 digits)
#define	IX_WEEK			((char)87)	// 'W'	Week Number
#define	IX_DAY			((char)89)	// 'Y'	Day Of Week
#define	IX_CALC			((char)90)	// 'Z'	Accumulated configurable 'Calculated' total
#define	IX_SONDTE		((char)91)	// '['	Module SignOn Date
#define	IX_SONDEP		((char)92)	//		Module SignOn Depot
#define	IX_SOFFDTE		((char)93)	// ']'	Module SignOff Date
#define	IX_CTLDATE		((char)94)	// '^'	Download Disc Control Date
#define	IX_SHDDUTY		((char)95)	//   	Scheduled Crew Duty (1-6 digits)
#define	IX_SOFFDEP		((char)96)	//		Module SignOFF Depot
#define	IX_SHDBUS		((char)97)	//   	Scheduled BusBoard (1-6 alphanumeric)
#define	IX_BRDREC		((char)98)	// 'b'	Board Stage record (one for each ETM Stage record)
#define	IX_SHDDATE		((char)99)	// 'c'	Scheduled Trip Date
#define	IX_SHDTIME		((char)100)	// 'd'	Scheduled Trip Time
#define	IX_SHDEND		((char)101)	// 'e'	Scheduled Trip EndTime
#define	IX_RAWFILE		((char)102)	// 'f'	Name of Raw Data ModuleFile
#define	IX_PSG			((char)103)	// 'g'	Accumulated Paid Passengers count
#define	IX_MONTH		((char)104)	// 'h'	Month (derived from Date)
#define	IX_TIMGRP		((char)105) // 'i'	TimeBand Group (requires schema)
#define	IX_JNYNDX		((char)106) // 'j'	Indexed Jny record (one for each ETM TripDeparture record)
#define	IX_MODFIL		((char)109)	// 'm'	Module FileName
#define	IX_SHDDEP		((char)110)	// 'n'	SCHEDULED Depot Number 0-59 from Trip Archive
#define	IX_DEPNAM		((char)111)	// 'o'	Depot Name
#define	IX_PRCGRP		((char)112)	// 'p'	Ticket Price Group (requires schema)
#define	IX_JNYPAX		((char)113)	// 'q'	Total Trip PaxCount
#define	IX_YEAR			((char)114)	// 'r'	Year (derived from Date)
#define	IX_PAS			((char)115)	// 's'	Accumulated Unpaid Pass count
#define	IX_JNYPOS		((char)116)	// 't'	Journey Record position within Module File
#define	IX_RAWDATE		((char)117)	// 'u'	OS Date/TimeStamp of Raw Data ModuleFile
#define	IX_REV			((char)118)	// 'v'	Accumulated Revenue
#define	IX_WALLET		((char)119)	// 'w'	Glasgow 'Ticket Wallet' Number
#define	IX_PAX			((char)120)	// 'x'	Accumulated Pax count
#define	IX_DAYGRP		((char)121)	// 'y'	Day Group (requires schema)
#define	IX_INSP			((char)122)	// 'z'	Inspector Number
#define	IX_SONTIM		((char)123)	// '{'	Module SignOn Time
#define	IX_SOFFTIM		((char)125)	// '}'	Module SignOff Time
#define	IX_HDRERV		((char)126)	// Module has HeaderControl REV discrepancy
#define	IX_HDRERX		((char)127)	// Module has HeaderControl PAX discrepancy
#define	IX_CLASSNAM		((char)128)	// WayClass Name
#define	IX_PRVCLASSNAM	((char)129)	// Previous Class Name
#define	IX_TICKETNUM	((char)130)	// TicketNumber 0-999999 (-1 if n/a)
//#define	IX_RAWFIL		((char)136)	// Original Raw FileName (ERG_Alt_Cnv, Way_LFN, etc.)
//#define	IX_RAWDATE		((char)137)	// Raw FileName Date/TimeStamp when loaded into archive
//#define	IX_ERGFIL		((char)138)	// ERG Raw (Alt_Cnv) FileName
#define	IX_STGTIME		((char)139)	// Stage Time
#define	IX_TRPEND		((char)141)	// ETM TripEndTime
#define	IX_TRPSPAN		((char)142)	// ETM Trip Duration in minutes
#define	IX_ERGETM		((char)144)	// ERG ETM Number
#define	IX_PRVSRV		((char)147)	// Previous ServiceNo
#define	IX_PRVJNY		((char)148)	// Previous JourneyNo
#define	IX_CONLINE		((char)154)	// Contract Line Number (UJI)
#define	IX_CON			((char)155)	// Contract Number
#define	IX_VALID_DCD	((char)156)	// Valid DriverConfirmedDeparture? (0=n/a, 1/2=N, 3/4=Y (2&4=First)
#define	IX_DCD_TIME		((char)157)	// Valid DriverConfirmedDepartTime of series (1st etm time if no valid DCD in series)
#define	IX_SHD_DEP		((char)158)	// Scheduled Trip Depot (normally EQV Operating Depot, see IX_SHD_LINK
#define	IX_SHD_TIME		((char)159)	// Scheduled Trip Depart Time
#define	IX_PRVSRVNUM	((char)164)	// Numeric part of Original ServiceNo
#define	IX_SRVNUM		((char)165)	// Numeric part of ServiceNo
#define	IX_CTLTIME		((char)170)	// Download Disc Control Time
#define	IX_SRVPREFX		((char)174)	// Alpha Prefix of ServiceNo (if any)
#define	IX_SRVSUFFX		((char)175)	// Alpha Suffix of ServiceNo (if any)
#define	IX_TKTYP		((char)179)	// Ticket Type 0-9999 (as per ERG)
#define	IX_PAXTYP		((char)180)	// Passenger Type
#define	IX_PAYTYP		((char)181)	// Payment Type
#define	IX_PAXNAM		((char)182)	// Passenger Type Name
#define	IX_COMTKT		((char)183)	// CommonTickType
#define	IX_COMNAM		((char)184)	// CommonTickTypeName

#define	IX_PAYNAM		((char)185)	// Payment Type Name
#define	IX_TKTNAM		((char)186)	// Ticket Type Name

#define	IX_MinLive		((char)191)	// Live TripTime		// 200707
#define	IX_MinDeadLay	((char)192)	// Dead TripTime + Layover		// 200707

#define	IX_PRVPRICE		((char)194)	// Previous Ticket Price
#define	IX_REVTODATE	((char)195)	// ETM Insertion Header "Total Revenue To Date"
#define	IX_ShdMil		((char)200)	// Scheduled Miles (Live+Dead)
#define	IX_AdjMil		((char)201)	// Net Mileage Adjustment (Live+Dead), (Gained-Lost)

// these 2 should be retired...
#define	IX_LOSTPOS		((char)202)	// +ve (Gained) Mileage Adjustments
#define	IX_LOSTNEG		((char)203)	// -ve (Lost) Mileage Adjustments

#define	IX_AdjREASON	((char)204)	// Lost/gained Mileage Adjustment Reason Code
#define	IX_ShdMilDead	((char)205)	// Dead Scheduled Miles
#define	IX_AdjMilDead	((char)206)	// Net Dead Mileage Adjustment (Gained-Lost)
#define	IX_STAGE			((char)207)	// 0=Standard StageCarriage, 1=Non-Stage
#define	IX_REGNO			((char)208)	// Traffic Commission "Notice & Proceedings" Registration Number (text string)
#define	IX_POF			((char)209)	// LostMileage Point of Failure
#define	IX_ShdMilLive	((char)210)	// Live Scheduled Miles
#define	IX_AdjMilLive	((char)211)	// Net Live Mileage Adjustment (Gained-Lost)
#define	IX_MAXJNYLEN	((char)212)	// Longest Journey Length (Live only) for current Serv
#define	IX_F6DATE		((char)213)	// Date of F6 record
#define	IX_F6TIME		((char)214)	// Time of F6 record
#define	IX_AdjMilLiveNeg ((char)215)	// Negative LIVE Mileage Adjustment (Lost)
#define	IX_AdjMilDeadNeg ((char)216)	// Negative DEAD Mileage Adjustment (Lost)
#define	IX_ORGDTE		((char)222)	// Original Insertion Date
#define	IX_SRVNAM		((char)223)	// Service Name
#define	IX_ALTNAM		((char)224)	// Alight Stage Name
#define	IX_BRDNAM		((char)225)	// Board Stage Name
#define	IX_CTL			((char)232)	// Disc Ctl_ID (incl. dupsecs)
#define	IX_INS			((char)233)	// Module Insertion Time
#define	IX_INS_END		((char)234)	// Module Insertion END Time
#define	IX_JNYREC		((char)236)	// Jny Instance (one for each ETM TripDeparture record)
#define	IX_PRVTYPE		((char)237)	// Previous Ticket Type
#define	IX_PRVCLASS		((char)238)	// Previous Ticket Class
#define	IX_PRVTKTNAM	((char)239)	// Previous Ticket Name
#define	IX_ETM_TRPCT	((char)240)	// Number of ETM Trips linked to Scheduled Trip
#define	IX_LATEDATE		((char)241)	// No. of days difference between Ctl-Date and Activity Date
#define	IX_LATEDEP		((char)242)	// Trip Start Late Departure in Minutes
#define	IX_LATEARR		((char)243)	// Trip End Late Arrival in Minutes
#define	IX_JNYID		((char)245)	// Unique ETM JnyID (increments from 1 for each TripDepart record scanned)
#define	IX_SHD_LINK		((char)246)	// Subscript into Daily Triplist
#define	IX_SHD_UF		((char)247)	// Uncertainty Factor of link into Daily Triplist

#define	IX_CV_TYP		((char)248)	// Contracted Vehicle Type (minimum trim level)
#define	IX_CV_CAP		((char)249)	// Contracted Vehicle Capacity (minimum seating capacity)
#define	IX_CV_EMS		((char)250)	// Contracted Vehicle Emissions (lowest acceptable Euro class 1-6)
#define	IX_EV_TYP		((char)251)	// Etm Vehicle Type (trim level)
#define	IX_EV_CAP		((char)252)	// Etm Vehicle Capacity (seating capacity)
#define	IX_EV_EMS		((char)253)	// Etm Vehicle Emissions

#define	IX_COMPANY		((char)254)	// CompanyNumber (in LOCATION.TGF, as used by wxp01 "LA Import")



// XCOM.CPP

extern	int rollover;
int	stripln(char *s, HDL f);
int	x2hm(const char *p);		// retrieve Hex-encoded wayfarer-style hhmm
int	x2hmv(const char *p);		// As x2hm() except ignore Rollover, and return NOTFND if hex value isn't a valid time
int	x2hm0(const char *p);		// as x2hm() but ignore rollover
int	a2hmv(const char *p);		// verifying version of above
int	a2hm2(int h, int m);
int	rollover_adjust(int hm);

short	c2bd(const char *dmy);		// short_bd from 6 char ddmmyy
int		c2bd2(short *bd, const char *from, const char *to);			// Convert 2 string dates to elements in passed array
int		x2bd(const char *xdmy);		// short_bd from 6 Hex nibbles ddmmyy
void	c2mask(char *mask60bits, const char *str_60_x_YN);
char	*mask2c(const char *mask60bits);	// returns ptr -> static 60 x Y/N (+ \0) character string
int		day_bin(char *day);
char	*day_str(int dy);
char	*dmy_stri(short bd);		// ddmmyy (no separator)
char	*dmy_stri2(short *bd);		// Format a DateRange from passed 2-element array into static buffer
char	*dmy_str(long dttm);		// ddmmyy (no separator)
char	*dmy_hm_str(long dttm);		// ddmmyy hhmm
char	*dmy_hm_dep_str(long dttm);	// ddmmyy hhmm.n
short	vb2bd(const char *vb);		// return short binary date from ccyy-mm-dd
short	week_start(short bd);		// return weekstart that bd falls within
void	build_daysop(int dop, char *w);
char	*dopterm2str(DOPTERM *dt);

char	e03str2dop(const char *str);	// 
char	*dop2e03str(char dop);


char	*unquote(char*);			// Remove quotes from string if present

void	get_dopterm(DOPTERM *dt, int bd);

void	clear_callbacks(void);
char	*findfile(char *fn);			// Find file 'fn' within the search path
const	char	*search_path0(void);			// Return first folder in the search path
										


#define	TIV_PATH		1	// Make 's' a valid path by adding \ if doesn't end with '\' or ':'.
#define	TIV_UPPER		2	// Convert 's' to Upper Case
#define	TIV_MULTI		4	// Read up to 'n' lines (n passed as the 1st char of 's', return
#define	TIV_SHORTPATH	8	// Convert 's' to 8.3 format if necessary (may be path OR filename)
#define	TIV_TAB			16	// Convert 's' from comma-separated list to tab-separated (always with TAB after last item)
//#define	TIV_SEMICOLON	32	// Allow string to contain semicolons

char	*ti_var(const char *varnam, char *contents, int opt);
int	ti_vari(const char *varnam, int deflt);
char	*ti_filespec(char *pth, const char *fn);
int	ti_var_y(const char *varnam);			// Return TRUE if TI-variable 'varnam' starts with Y or y
int	ti_var_y_once(int *flag, char *ti_name);	// Only read the setting ONCE (it's initially set to NOTFND)

HDL	flopen_search(char *fn);
void	default_extn(char *p, const char *xtn);

void	dynag2cmpnam(DYNAG *d);
int	flgetln_trim(char *str, int bufsz, HDL f);
int	flgetln_strip(char *str, HDL f);

ulong	encode(char kn, const char *str);	// InsKey string to binary conversion (fld=IX_xxx)
char	*expand(char fld,ulong ky);			// InsKey binary to string conversion (uses 4 static areas)


int	srv_is_valid1(char c);
int	srv_is_valid(const char *s);

int	_cdecl cp_etm(const char *a, const char *b);
void	item_del(void *table, int table_ct, int item_to_del, int item_sz);



// Generic progress reporter used where app is concerned with a subtask preceded and/or followed by other subtasks
// Values passed as "percentage" should be in range 0-100.
// pre_subtask=percentage of "total task" already done by a previous routine
// post_subtask=percentage of "total task" that will be done by a following routine
// report() calculates "total task" percentage by normalising done, then calls the standard Status reporter
class PROGRESS {
public:
PROGRESS(int pre_subtask, int post_subtask);	// Percentages of "Total Task" before/after the current subtask
void	report(int done, int tot=NOTFND);
private:
int		pre,post,subtask_max, total, last_percent;
};


void GetTitle(char *buf, char *title);

int		wanted_fld(char typ,char *n, char *nxt=0);

int _cdecl cp_daysop(const char *a, const char *b);

struct SY;			// (pre-declare SY as being a structure)
void	global_init(const char *main_pth);
int	global_closedown(void);

class LOGTIMER
{
public:
LOGTIMER(char *_txt);
~LOGTIMER();
private:
char *txt;
HDL tmr;
};

void	set_search_path(const char *pth);

HDL	open_log_file(char *process_description);	// Open (append to existing) TI.log file in first 'Search_path' folder


int __stdcall	DllSuppress(char *suppressed_fields);
extern	char	ix_suppress[];

int	hex2buff(char *hex);

void	list_mnemonics(void);

int	get_1_dep(const char *mask);	// returns 0-59 being the ONE bitflag set in mask, else crashes

int	contract_end(const char *contract);		// Return YES if contract is empty string or '0'

int	sy_rng_len(SY *s);

