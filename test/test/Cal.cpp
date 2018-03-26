#include "pdefs.h"
#include <time.h>


char *cal2VbDate(long cal)
{
static int sw=0;						// can include up to 4 dates in 1 call
static char static_str[4][16];
char *str=static_str[sw++&3];
calfmt(str,"%02D-%3.3M-%4C",cal);
return(str);
}
char *cal2Vb1Date(long cal, bool start)
{
static int sw=0;						// can include up to 4 dates in 1 call
static char static_str[4][16];
char *str=static_str[sw++&3];
if (start)
	calfmt(str,"%02D-%3.3M-%4C 23:59:59",cal);
else
	calfmt(str,"%02D-%3.3M-%4C 00:00:00",cal);
return(str);
}


/*
A date/time stamp is the number of seconds before (if negative) or after (if positive) January 1, 1970 at 12:00:00 midnight.
The system only works for years in the range 1939-2037 - century is assumed if a 2-digit year as passed to Caljoin,
but IT'S NOT RELIABLE for '38, even if century is explicitly passed.

Midnight belongs to the new day.

calerr() - set after a call to Caljoin() - returns:
	0		no error
	1		error in year		year < 100   or   1920 <= year <= 2019
	2		error in month		1 <= month <= 12
	3		error in day		1 <= day <= # in mo.
	4		error in hour		0 <= hour <= 23
	5		error in minute	0 <= minute <= 59
	6		error in second	0 <= second <= 59

Numeric conversions:
	%C	century and year (4 digit year)
	%Y	year	(2 digit year)
	%O	month
	%D	day
	%T	hour	(24 hour clock)
	%H	hour	(12 hour clock)
	%I	minute
	%S	second

string conversions:
	%M	month name as text
	%W	weekday as text
	%P	part of day (AM, PM, etc.) as text

The usual strfmt options for %d and %s work here for numerics and 
strings, too. For example, "%02D" will produce a two-digit zero filled 
number for the day, "%-4.3W" will produce the first 3 characters of the 
weekday left-adjusted in a 4 character field.  Lower case conversion
specifiers are OK.

Some popular date formats:
	02 Jun 1985			"%02D %.3M %C"
	January 3, 1985		"%M %D, %C"
	Friday, January 5	"%W, %M %D"
	12/05/85			"%02O/%02D/%02Y"

Some popular time formats:
	23:12:02			"%02T:%02I:%02S"
	9:46A				"%H:%02I%.1P"
	02:42 PM			"%02H:%02I %P"
*/


#define FOURYRSECS  ( (4L*365L + 1L) * DAY_SECS )

#define BASEYR      1970      /* calnow() is 0 on 1/1/1970 at midnight */
#define WDBASEYR       4      /* day of week, Jan. 1, BASEYR           */
#define MAXYRS        68      /* # of whole years in 2**31 seconds     */

#define NLEAPS(ryr)  (   ( ((ryr)+BASEYR-1)>>2 ) - ( (BASEYR-1)>>2 )   )

static	int _calerr, _csecs;

static	int  _mth_tab(int mth,int yr)
{
static	short _ml[] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
return(_ml[mth-1]+ (mth>=3 && !(yr&3)));
}

int	calerr(void)
{return( _calerr );}

static int  calmthlen(int mth,int yr)
{return(_mth_tab(mth+1,yr) - _mth_tab(mth,yr));}


long	caljoin(int yr,int mth,int day,int hr,int mi,int sc)
{
int	cyr;
_calerr=0;
if (yr<100) yr+=((yr>38)?1900:2000);
cyr=yr;		// Save the year inclusive of century, for leap year tests
if ((yr-=BASEYR)<-MAXYRS || yr>MAXYRS) _calerr=CE_YR;
else if (mth<1 || mth>12 ) _calerr=CE_MTH;
else if (day<1 || day>calmthlen(mth,cyr)) _calerr=CE_DAY;
else if (hr<0 || hr>23) _calerr=CE_HR;
else if (mi<0 || mi>59) _calerr=CE_MIN;
else if (sc<0 || sc>59) _calerr=CE_SEC;
else return(DAY_SECS*(365L*yr+NLEAPS(yr)+day+_mth_tab(mth,cyr)-1L)
			+hr*3600L+mi*60L+sc);
return(0);
}

long	calnow(void)
{
time_t t;
return((long)time(&t));
}

#include "sys/types.h"
#include "sys/timeb.h"

void calsplit(long cal, short *pyr,short *pmth,short *pday,short *phr,short *pmi,short *psc)
{
int	yr, mth, day;
yr   = BASEYR + 4 * (cal/FOURYRSECS);	// take care of 4 year periods
cal %= FOURYRSECS;						// and ensure that cal is positive
if  (cal<0) {cal+=FOURYRSECS; yr-=4;}
if  (psc) *psc = short(cal % 60L); cal /= 60L;		// seconds
if  (pmi) *pmi = short(cal % 60L); cal /= 60L;		// minutes
if  (phr) *phr = short(cal % 24L); cal /= 24L;		// get hours

day = short(cal);	/* cal is now No. of days since 1/1/BASEYR */
while (day>=_mth_tab(13,yr))
	day-=_mth_tab(13,yr++); // mth 13 is Jan next yr
mth=1;
while (day>=_mth_tab(mth+1,yr))
	mth++;

if (pyr) *pyr=(short)yr;
if (pmth) *pmth=(short)mth;
if (pday) *pday=(short)(day+1-_mth_tab(mth,yr));
}



void cal_check_date_range(short bd0, short bd1)
{
if (bd1<bd0 || bd0<BD1980) m_finish("Internal Error: Bad Date Range");
}


#define H_TMR ((_TMR*)tmr)
void tmrreset(HDL tmr,long csecs)
{
clock_t ticks=clock();
ticks+=((csecs*100L)/CLOCKS_PER_SEC);
H_TMR->secs=ticks/CLOCKS_PER_SEC;
H_TMR->ticks=ticks%CLOCKS_PER_SEC;
}


/* make a timer and set it */
HDL	tmrist(long csecs)
{
HDL tmr=(HDL)memgive( sizeof(_TMR) );
tmrreset (tmr, csecs );
return(tmr);
}


/* get rid of a timer */
void tmrrls(HDL tmr)
{memtake(tmr);}

/* check how much time has elapsed since set point */
long	tmrelapsed(HDL tmr)		/* returns negative number if before */
{
clock_t ticks=clock();

//ticks=clock();ticks=clock();ticks=clock();

//int g=CLOCKS_PER_SEC;

long secs=ticks/CLOCKS_PER_SEC - H_TMR->secs;
long csecs=((ticks%CLOCKS_PER_SEC)*100L)/CLOCKS_PER_SEC;
return(secs*100L + csecs);
}


#ifdef not_used
long	calnow100(void)
{
struct _timeb t;
_ftime(&t);
int now=t.time*100;
now+=(t.millitm/10);
return(now);
}

/***
* Name:         calframe -- adjust date stamps to period boundaries
*
* Synopsis:     int calframe();
*               int nprds, prdtypp;
*               long cal_beg, cal_end;
*
*               nprds = calframe( prdtypp, &cal_beg, &cal_end );
*
* Description:  Adjusts CAL_BEG and CAL_END so that they fall on period
*               boundaries, and returns the number of periods between the
*               adjusted dates.  PRDTYPP determines the type of period:
*               workdays, weeks, months, quarters, or years.  CAL_BEG is
*               rounded down to the nearest period, CAL_END is rounded up.
*
* Returns:      The number of periods between the adjusted dates.
*
* Notes:        This routine depends on Caljoin() making sense out of a
*               date like 4/-2/80.  I.e., the day part may be < 1, or 
*               greater than the number of days in the month, and Caljoin()
*               must do the sensible thing.
	$$$$$$$$$$$$ 22/11/95 caljoin() now treats month<1 or > 12 as month=1
***/
ushort calframe(int prdtypp,long *pcal_beg,long *pcal_end)
{
short 	wkday, yr1, mth1, day1, yr2, mth2, day2, i;
long	end;
ushort	ndays;
calsplit(*pcal_beg,&yr1,&mth1,&day1,0,0,0);
calsplit(*pcal_end,&yr2,&mth2,&day2,0,0,0);
switch (prdtypp)
	{
   case CAL_WKDAYS:
    	wkday=calwkday(*pcal_beg);
      if  (wkday==0)	i=1; /* SUNDAY */
		else if (wkday==6) i=2;	/* SATURDAY */
		else i=1-wkday;
		*pcal_beg += ((long) i * DAY_SECS);
      calsplit(*pcal_beg, &yr1, &mth1, &day1,0,0,0);
		if	(*pcal_end < *pcal_beg) *pcal_end = *pcal_beg;
		/* fall through ok */
   case CAL_WKS:
		ndays = (int)((*pcal_end - *pcal_beg)/DAY_SECS);
		i = (int)((ndays%7) ? 1 : 0);
		end = *pcal_beg + (((ndays/7L)+i)*7L*DAY_SECS);
		calsplit( end, &yr2, &mth2, &day2,0,0,0);
      break;
   case CAL_MTHS:
      day1 = 1;
      if  ( day2 > 1 ) mth2++;
      day2 = 1;
      break;
   case CAL_QTRS:
      mth1 = (((mth1-1)/3)*3)+1;
      day1 = 1;
		if	(day2 > 1) mth2++;
		i = (mth2%3) ? 1 : 0;
      mth2 = ((((mth2-1)/3)+i)*3)+1;
      day2 = 1;
      break;
   case CAL_YRS:
      mth1 = day1 = 1;
      if  ( mth2 > 1 || day2 > 1 ) yr2++;
      mth2 = day2 = 1;
      break;
	}
if (mth2>12) {mth2-=12; yr2++;}
*pcal_beg = caljoin( yr1, mth1, day1, 0, 0, 0 );
*pcal_end = caljoin( yr2, mth2, day2, 0, 0, 0 );
return(calnprds(prdtypp,*pcal_beg,*pcal_end) );
}
#endif
const	char*	sy_mthnam[]={"January","February","March","April","May","June",
					"July","August","September","October","November","December"};
const	char*	sy_daynam[]={"Sunday","Monday","Tuesday","Wednesday","Thursday",
					"Friday","Saturday"};

static	const char	*sy_dayprt[]={"AM","PM"};


static int  calwkday(long cal)
{
int	wkday=( cal/DAY_SECS + WDBASEYR ) % 7;
if	(wkday < 0) wkday += 7;
return(wkday);
}


char	*calfmt(char *str, char *fmt, long cal)
{
int		p,out_pos=0,in_pos=0,ch,i=0;
short	yr,mo,dy,hr,mn,se;
const	char *s;
char	c,*ctl=stradup(fmt);
calsplit(cal, &yr, &mo, &dy, &hr, &mn, &se);
while ((p=stridxc('%',&ctl[in_pos]))!=NOTFND)
	{
	if ((ch=strinspn("CcYyOoDdTtHhIiSsMmWwPp",&ctl[p+=in_pos]))==NOTFND)
		{str[out_pos++]=ctl[in_pos++];continue;}
	s=NULL;
	c=ctl[p+=ch];
	switch (TOUPPER(c))
		{
		case 'C': i=yr;break;
		case 'Y': i=yr%100;break;
		case 'O': i=mo;break;
		case 'D': i=dy;break;
		case 'T': i=hr;break;
		case 'H': i=hr%12;break;
		case 'I': i=mn;break;
		case 'S': i=se;break;
		case 'M': s=sy_mthnam[mo-1];break;
		case 'W': s=sy_daynam[calwkday(cal)];break;
		case 'P': s=sy_dayprt[hr>=12];break;
		}
   ctl[p]='s'; if (!s) {ctl[p]='d'; s=(char*)((long)i);}
   c=ctl[++p];	/* advance pointer in ctl-string to next item */
   ctl[p]=0;	/* truncate ctl-string to format just this item */
   out_pos+=strlen(strfmt(&str[out_pos],&ctl[in_pos],s));
   ctl[in_pos=p]=c;	/* restore truncated ctl-string */
   }
strcpy(&str[out_pos],&ctl[in_pos]);	/* any non-control string left over? */
memtake(ctl);
return(str);
}

#ifdef Unused
char** calfmtset(int type,char** txt_ptrs)
{
switch (type)
	{
	case 0: _mn=_mnames; _dn=_dnames; return(NULLPTR);
	case 1: if (txt_ptrs) _mn=txt_ptrs;		return(_mn);
	case 2: if (txt_ptrs) _dn=txt_ptrs;		return(_dn);
	}
}

/***
* Name:         calnprds -- determines the number of periods between 2 dates
*
* Synopsis:     int calnprds();
*               int nprds, prdtypp;
*               long cal_beg, cal_end;
*
*               nprds = calnprds( prdtypp, cal_beg, cal_end );
*
* Description:  PRDTYPP determines a period type: workdays, weeks, months
*               quarters, or years.  Calnprds() returns the number of whole
*               periods of the given type between two date/time stamps.
*
* Returns:      The number of periods between the dates.
***/

ushort calnprds(int prdtypp,long cal_beg,long cal_end)
{
static  char    _table[7][7] =
    {   { 0, 0, 1, 2, 3, 4, 5 },
        { 5, 0, 1, 2, 3, 4, 5 },
        { 4, 4, 0, 1, 2, 3, 4 },
        { 3, 3, 4, 0, 1, 2, 3 },
        { 2, 2, 3, 4, 0, 1, 2 },
        { 1, 1, 2, 3, 4, 0, 1 },
        { 0, 0, 1, 2, 3, 4, 0 }
    };
ushort	nprds, ndays, nmths;
short	yr1, mth1, day1, yr2, mth2, day2;
if	(cal_end < cal_beg) {long w=cal_end; cal_end=cal_beg; cal_beg=w;}
if	(prdtypp <= CAL_WKS)
	{
	ndays = (ushort)((cal_end - cal_beg) / DAY_SECS);
	switch (prdtypp)
	   {
		case CAL_DAYS: nprds = ndays; break;
		case CAL_WKDAYS:
	   	nprds = (ndays/7)*5 + _table[calwkday(cal_beg)][calwkday(cal_end)];
         break;
		case CAL_WKS: nprds = ndays / 7; break;
	   }
	}
else
	{
	calsplit(cal_beg,&yr1,&mth1,&day1,0,0,0);
	calsplit(cal_end,&yr2,&mth2,&day2,0,0,0);
	if  (mth2 < mth1) {mth2 += 12; yr2--;}
	nmths = mth2 - mth1;
	switch (prdtypp)
	   {
		case CAL_MTHS: nprds = 12*(yr2 - yr1) + nmths;	break;
		case CAL_QTRS: nprds = nmths/3 + ((nmths%3)!=0) + 4*(yr2 - yr1);	break;
		case CAL_YRS:  nprds = yr2 - yr1; break;
	   }
	}
return(nprds);
}
#endif
