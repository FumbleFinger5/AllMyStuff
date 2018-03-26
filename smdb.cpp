//#include "xlib.h"
//#include "dgrp.h"
#include <stdio.h>
#include "WXP07.H"

extern char epth[200];


struct	EM_KEY {char nam[60]; short year;};
struct	EM_REC {EM_KEY ky; ushort sz[16];};

static int _cdecl cp_em_key(EM_KEY *a, EM_KEY *b)
{
int cmp=stricmp(a->nam,b->nam);
if (!cmp) cmp=cp_short(&a->year,&b->year);
return(cmp);
}


void dbsafeclose(HDL db);

//static	SMDB	*smdb;

void SMDB::db_open(char *fn)
{
//int prv=dbsetlock(YES);
if ((db=dbopen(fn))==NULLHDL) m_finish("Error opening %s",fn);
recget(db,dbgetanchor(db),&hdr,sizeof(hdr));						// Read anchor record
if ( !hdr.ver			// No version?
||  (em_btr=btropen(db,hdr.em_rhdl))==NULLHDL)
	m_finish("Error reading %s",fn);
btr_set_cmppart(em_btr,(PFI_v_v)cp_em_key);
//dbsetlock(prv);
}


SMDB::SMDB(char *fn)
{
if (!drattrget(fn,0))
	{
	if (!dbist(fn) || (db=dbopen(fn))==NULLHDL) goto err;
	memset(&hdr,0,sizeof(hdr));
	hdr.ver=1;
	hdr.em_rhdl=btrist(db,DT_USR,sizeof(EM_REC));
	dbsetanchor(db,recadd(db,&hdr,sizeof(hdr)));
	dbsafeclose(db);
	}
db_open(fn);
return;
err:
m_finish("Error creating %s",fn);
}


static int get_year(char *n)	// Get bracketed year from end of moviename (& delete from passed string)
{
int z=strlen(n), yr;
if (n[z-1]!=')' || n[z-6]!='(' || n[z-7]!=SPACE || (yr=a2i(&n[z-5],4))<1910 || yr>2032)
	m_finish("internal year error");
n[z-7]=0;	// truncate passed "foldername" to become "moviename" without year
return(yr);
}

void SMDB::zap(int slot) // If slot==NOTFND, delete any em_btr keys with all locn/sz values=0, else zeroise [slot]
{
RHDL rh=0;
EM_REC e;
memset(&e,0,sizeof(EM_REC));
while (bkysrch(em_btr,BK_GT,&rh,&e))
	if (slot!=NOTFND)
		{
		e.sz[slot]=0;
		bkyupd(em_btr,&rh,&e);
		}
	else
		{
		int i, fnd;
		for (i=fnd=0; !fnd && i<16; i++) if (e.sz[i]) fnd=YES;
		if (!fnd) bkydel_rec(em_btr,&e);
		}
}

void SMDB::upd(int slot, DYNAG *em)
{
int i;
zap(slot);	// FIRST zeroise existing "folder size" values for the current slot (location)
for (i=0;i<em->ct;i++)
	{
	CMFC *c=(CMFC*)em->get(i);
	EM_REC e;
	memset(&e,0,sizeof(EM_REC));
	e.ky.year=get_year(c->mn);
	strcpy(e.ky.nam,c->mn);
	RHDL rh=0;
	if (bkysrch(em_btr,BK_EQ,&rh,&e))
		recget(db,rh,e.sz,sizeof(e.sz));
	else
		bkyadd(em_btr,rh=recadd(db,e.sz,sizeof(e.sz)),&e.ky);
	e.sz[slot]=c->sz;
//if (slot==8) e.sz[9]=0;
	recupd(db,rh,e.sz,sizeof(e.sz));
	}
zap(NOTFND);	// FINALLY - delete any keys where "folder size" is zero for ALL slots/locations
				// note - this means there won't be any record if movie ONLY exists in emdb.csv
//test();
}

void SMDB::test(void)
{
int i,j, dup, hi, again=NO;
RHDL rh;
EM_REC e;
void *empty=memgive(sizeof(e.sz));
char lin[200];
while (bkyscn_all(em_btr,&rh,&e,&again))
	{
	recget(db,rh,e.sz,sizeof(e.sz));
	for (i=hi=dup=0;i<16;i++)
		if (e.sz[i]) {dup++; if (e.sz[i]>hi) hi=e.sz[i];}
	strfmt(lin,"%d ",dup);
	for (i=0;i<16;i++)
		{
		char c=SPACE; if (e.sz[i]) c=((e.sz[i]==hi)?'X':'x');
		strendfmt(lin,"%c",c);
		}
	strendfmt(lin,"  %s (%04.4d)",e.ky.nam,e.ky.year);
	sjhlog("%s",lin);

/*	if (memcmp(e.sz,empty,sizeof(e.sz)))
		{
		printf("%s (%04.4d) ",e.ky.nam,e.ky.year);
		for (i=0; i<16; i++) printf(" %4.4d",e.sz[i]);
		printf("\r\n");
		}*/
	}
i=i;
memtake(empty);
}

void SMDB::test3(void) // list "number of movies not on 04Kraken+06Kingkong"
{
int i,j, dup, hi, again=NO;
int *ct=(int*)memgive(12*sizeof(int));
RHDL rh;
EM_REC e;
char lin[200];
while (bkyscn_all(em_btr,&rh,&e,&again))
	{
	recget(db,rh,e.sz,sizeof(e.sz));
	if (e.sz[3]!=0 && e.sz[5]!=0) continue;	// SAFE - stored in duplicate on 04Kraken+06Kingkong
	if (e.sz[1]!=0 && e.sz[2]!=0) continue;	// SAFE - stored in duplicate on 02SH+03Magnum
//	for (i=0;i<12;i++) if (e.sz[i]) ct[i]++;
	if (e.sz[9])
		{
		strfmt(lin,"%s (%04.4d)",e.ky.nam,e.ky.year);
//		sjhlog("%s",lin);
		sjhlog("XCOPY %cG:\\newfilms\\%s%c %cF:\\newfilms\\%s%c /E /I",34,lin,34,34,lin,34);
		}
	}
//for (i=0;i<12;i++) 	sjhlog("Disc:%2.2d %9d",i+1,ct[i]);
memtake(ct);
}

void SMDB::test2(void)
{
int i,j, dup, hi, again=NO;
RHDL rh;
EM_REC e;
char lin[200];
int *ct=(int*)memgive(12*sizeof(int));
while (bkyscn_all(em_btr,&rh,&e,&again))
	{
	recget(db,rh,e.sz,sizeof(e.sz));
	if (e.sz[3]!=0 && e.sz[5]!=0) continue;	// SAFE - stored in duplicate on 04Kraken+06Kingkong
	if (e.sz[3]!=0 || e.sz[5]!=0) m_finish("Ooops");
	for (i=0;i<12;i++) if (e.sz[1]==0) if (e.sz[i])
		if (i==2)
		{
		ct[i]++;
strfmt(lin,"%s (%04.4d)",e.ky.nam,e.ky.year);
//		sjhlog("XCOPY %cF:\\newfilms\\%s%c %cE:\\newfilms\\%s%c /E /I",34,lin,34,34,lin,34);
		sjhlog("REN %cF:\\FilmsX\\_%s%c %c%s%c",34,lin,34,34,lin,34); // #### <<<=== this doesn't work!!!
		}
	}
//for (i=0;i<12;i++) 	sjhlog("Disc:%2.2d %9d",i+1,ct[i]);
memtake(ct);
}

void SMDB::test1(void)
{
int i,j, dup, hi, again=NO;
RHDL rh;
EM_REC e;
char lin[200];
long *sz=(long*)memgive(12*sizeof(long));
while (bkyscn_all(em_btr,&rh,&e,&again))
	{
	recget(db,rh,e.sz,sizeof(e.sz));
for (i=0;i<12;i++) sz[i]+=e.sz[i];
//if (e.sz[0] || e.sz[1] || e.sz[2]) i=i;
//	if (e.sz[3]==0 && e.sz[5]==0) continue;	// SAFE - stored in duplicate on 04Kraken+06Kingkong
//	if (e.sz[4]==0 && e.sz[6]!=0)	// on 07Jimbob but not 08Godzilla
/* if (e.sz[4]!=0 && e.sz[3]!=0)	// SAFE but also on 05Godzilla
		{
		strfmt(lin,"%4.4d %4.4d",e.sz[3],e.sz[5]);
		strendfmt(lin,"  %s (%04.4d)",e.ky.nam,e.ky.year);
//	if (e.sz[3]!=e.sz[5]) strendfmt(lin,"%s"," ####");
		sjhlog("%s",lin);
		}
*/
	}
for (i=0;i<12;i++)
	sjhlog("%2.2d %9d",i,sz[i]);
memtake(sz);
}

int update_dbf(int disc_num, char *disc_pth, DYNAG *em)
{
char buf[1000];
strfmt(buf,"%s\\%s",epth,"SMDB.grp");
HDL f=flopen(buf,"r+");
short slot=NOTFND, i;
for (i=0; flgetln(buf,100,f)>0; i++)
	if (a2i(buf,2)==disc_num && !stricmp(&buf[2],disc_pth))
		{slot=i;break;}
if (slot==NOTFND)
	{
	if ((slot=i)>=15) m_finish("Too many Disc+Path slots");
	strfmt(buf,"%02.2d%s",disc_num,disc_pth);
	flputln(buf,f);
	printf("Appended Disc:%02.2d Path:%s to SMDB.grp as Slot:%d\r\n",disc_num,disc_pth,slot);
	}
flclose(f);
strfmt(buf,"%s\\%s",epth,"SMDB.dbf");
SMDB s(buf);
s.upd(slot, em);
return(0);
}
