#include "xlib.h"
#include "dgrp.h"
#include "xsq.h"


static const SY StdSY[]={	// typ         vbt len maxl  sdig maxp
"Company",		"COMPANY",		254,		IX_COMPANY,		'C',1,3,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"CtlDate",		"CTLDATE",		BIGDATE,	IX_CTLDATE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm|F_Date,
"CtlTime",		"CTLTIME",		BIGTIME,	IX_CTLTIME,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"DiscID",		"CTL",			BIGDATL,	IX_CTL,			'C',4,18,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,
"Depot",		"DEPNUM",		59,			IX_DEPNUM,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,
"DepNam",		"DEPNAM",		0,			IX_DEPNAM,		'C',1,20,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_RtJst,
"Filename",		"MODFIL",		99999999,	IX_MODFIL,		'C',4,11,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_LfJst,
"ErgM/c",		"ERGETM",		0xFFFFFFFF,	IX_ERGETM,		'H',4,8,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"Module",		"MODULE",		999999,		IX_MODULE,		'N',4,7,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,
//"ERG FNam",		"ERGFIL",		0xFFFFFFFF,	IX_ERGFIL,		'H',4,8,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"S-OnDepot",	"SONDEP",		59,			IX_SONDEP,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"S-OffDep",		"SOFFDEP",		59,			IX_SOFFDEP,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"S-OnDte",		"SONDTE",		BIGDATE,	IX_SONDTE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Date,
"S-OffDte",		"SOFFDTE",		BIGDATE,	IX_SOFFDTE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Date,
"S-OnTim",		"SONTIM",		BIGTIME,	IX_SONTIM,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"S-OffTim",		"SOFFTIM",		BIGTIME,	IX_SOFFTIM,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,

"RawFile",		"RAWFILE",		0,			IX_RAWFILE,		'C',2,40,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_LfJst|F_IsTxt,
"RawDate",		"RAWDATE",		BIGDATL,	IX_RAWDATE,		'C',4,18,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,

"Machine",		"ETM",			SQ6xZ,		IX_ETM,			'C',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,
"Vehicle",		"VEHICLE",		SQ6xZ,		IX_VEHICLE,		'C',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,

"EtmVTyp",		"EV_TYP",		0,			IX_EV_TYP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"EtmVCap",		"EV_CAP",		0,			IX_EV_CAP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"EtmVEms",		"EV_EMS",		0,			IX_EV_EMS,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,

"Driver",		"DRIVER",		9999999,	IX_DRIVER,		'N',4,7,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,
"Duty",			"DUTY",			SQ6xZ,		IX_DUTY,		'N',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,
"RevToDate",	"REVTODATE",	0xFFFFFFFF,	IX_REVTODATE,	'N',4,15,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"Date",			"DATE",			BIGDATE,	IX_DATE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_OkScm|F_Date,
"Days of Op",	"DAY",			127,		IX_DAY,			'C',1,7,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_OkScm|F_LfJst,
"DayGroup",		"DAYGRP",		0,			IX_DAYGRP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkScm,
"WeekNo",		"WEEK",			999,		IX_WEEK,		'N',2,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_OkScm|F_RtJst,
"Month",		"MONTH",		12,			IX_MONTH,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_OkScm|F_RtJst,
"Year",			"YEAR",			99,			IX_YEAR,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_OkScm|F_RtJst,
"OrgDate",		"ORGDTE",		BIGDATE,	IX_ORGDTE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"LateS/O",		"LATEDATE",		BIGDATE,	IX_LATEDATE,	'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"Trip End",		"TRPEND",		BIGTIME,	IX_TRPEND,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_Time,
"Direction",	"DOT",			3,			IX_DOT,			'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_OkScm,
"Jrev",			"JNYREV",		999999,		IX_JNYREV,		'N',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkRng,
"Jpax",			"JNYPAX",		9999,		IX_JNYPAX,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkRng,
"HdrErrRev",	"HDRERV",		1,			IX_HDRERV,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,
"HdrErrPax",	"HDRERX",		1,			IX_HDRERX,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,

"Valid DCD",	"VALID_DCD",	4,			IX_VALID_DCD,	'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,
"DCD Time",		"DCD_TIME",		BIGTIME,	IX_DCD_TIME,	'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_Time,
"F6Date",		"F6DATE",		BIGDATE,	IX_F6DATE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Date,
"F6Time",		"F6TIME",		BIGTIME,	IX_F6TIME,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"Service",		"SRV",			SQ4xZ,		IX_SRV,			'C',4,4,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,
"SrvPrefix",	"SRVPREFX",		SQ3xZ,		IX_SRVPREFX,	'C',4,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"SrvSuffix",	"SRVSUFFX",		SQ3xZ,		IX_SRVSUFFX,	'C',4,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"ServName",		"SRVNAM",		0,			IX_SRVNAM,		'C',2,24,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_LfJst|F_IsTxt,

"InsTime",		"INS",			BIGTIME,	IX_INS,			'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_Time,
"InsEnd",		"INS_END",		BIGTIME,	IX_INS_END,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"ServNum",		"SRVNUM",		9999,		IX_SRVNUM,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"OldSrv",		"PRVSRV",		SQ4xZ,		IX_PRVSRV,		'C',4,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"OldSrvNum",	"PRVSRVNUM",	9999,		IX_PRVSRVNUM,	'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"OldJny",		"PRVJNY",		9999,		IX_PRVJNY,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"JnyREC",		"JNYNDX",		0xFFFFFFFF,	IX_JNYNDX,		'C',4,20,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_LfJst,
"Jrny",			"JOURNEY",		9999,		IX_JOURNEY,		'N',2,4,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,
"TripCt",		"ETM_TRPCT",	255,		IX_ETM_TRPCT,	'I',1,3,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"JnyInstance",	"JNYREC",		0,			IX_JNYREC,		'N',4,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey,
"Trip Time",	"TIME",			BIGTIME,	IX_TIME,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_Time,
"J-ID",			"JNYID",		0,			IX_JNYID,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey,
"Schd Date",	"SHDDATE",		BIGDATE,	IX_SHDDATE,		'D',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Date,
"Schd Time",	"SHDTIME",		BIGTIME,	IX_SHDTIME,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"LateStart",	"LATEDEP",		9999,		IX_LATEDEP,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Sign,
"ShdEndTim",	"SHDEND",		BIGTIME,	IX_SHDEND,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"LateEnd",		"LATEARR",		9999,		IX_LATEARR,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Sign,
"ShdTripSub",	"SHD_LINK",		9999,		IX_SHD_LINK,	'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,
"ShdUF",		"SHD_UF",		255,		IX_SHD_UF,		'N',1,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"ShdTime",		"SHD_TIME",		BIGTIME,	IX_SHD_TIME,	'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_Time,
"JnySpan",		"TRPSPAN",		999,		IX_TRPSPAN,		'N',2,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PointOfFail",	"POF",			0,			IX_POF,			'C',2,POF_IDLEN,1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_LfJst|F_IsTxt,
"ShdDuty",		"SHDDUTY",		SQ6xZ,		IX_SHDDUTY,		'N',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"ShdBus",		"SHDBUS",		SQ6xZ,		IX_SHDBUS,		'C',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"ShdDep",		"SHDDEP",		59,			IX_SHDDEP,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_RtJst,
"Non-Stage",	"STAGE",		1,			IX_STAGE,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"RegNo",		"REGNO",		0,			IX_REGNO,		'C',2,NP_IDLEN,1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst|F_IsTxt,
"LostReason",	"AdjREASON",	255,		IX_AdjREASON,	'N',1,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"JnyLen",		"MAXJNYLEN",	0,			IX_MAXJNYLEN,	'I',2,0,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_RtJst|F_Miles,

"Contract",		"CON",			0,			IX_CON,			'C',2,CON_IDLEN,1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng|F_LfJst|F_IsTxt,
"ConLine",		"CONLINE",		9999999,	IX_CONLINE,		'N',4,7,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_Tkx|F_OkRng,	// TKX4
"ConVTyp",		"CV_TYP",		16,			IX_CV_TYP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"ConVCap",		"CV_CAP",		255,		IX_CV_CAP,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,
"ConVEms",		"CV_EMS",		99,			IX_CV_EMS,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_OkScm,

"BrdInstance",	"BRDREC",		9999,		IX_BRDREC,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"Brd Stage",	"BRDNUM",		9999,		IX_BRDNUM,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"RunBrd",		"RUNBRD",		0,			IX_RUNBRD,		'N',2,0,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,    // Running_order
"Board",		"BRDNAM",		0,			IX_BRDNAM,		'C',2,30+3,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_LfJst|F_IsTxt,

"StageTime",	"STGTIME",		BIGTIME,	IX_STGTIME,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"TktTime",		"TKTTIM",		9999999,	IX_TKTTIM,		'T',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_Time,
"Inspector",	"INSP",			99999999,	IX_INSP,		'N',4,8,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PrvPrc",		"PRVPRICE",		32000,		IX_PRVPRICE,	'N',2,5,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PrvTTyp",		"PRVTYPE",		238,		IX_PRVTYPE,		'H',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PrvCla",		"PRVCLASS",		0xE,		IX_PRVCLASS,	'H',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PrvClNam",		"PRVCLASSNAM",	0,			IX_PRVCLASSNAM,	'C',2,TKTNAM_SZ,1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_RtJst,
"PrvNam",		"PRVTKTNAM",	0,			IX_PRVTKTNAM,	'C',2,TKTNAM_SZ,1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_RtJst,
"TktTyp",		"TKTYP",		9999,		IX_TKTYP,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"TktNam",		"TKTNAM",		0,			IX_TKTNAM,		'C',2,10,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_LfJst,
"PaxTyp",		"PAXTYP",		99,			IX_PAXTYP,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PaxNam",		"PAXNAM",		0,			IX_PAXNAM,		'C',2,10,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_LfJst,
"PayTyp",		"PAYTYP",		99,			IX_PAYTYP,		'N',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PayNam",		"PAYNAM",		0,			IX_PAYNAM,		'C',1,10,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_LfJst,
"ComTkt",		"COMTKT",		255,		IX_COMTKT,		'N',1,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"ComNam",		"COMNAM",		255,			IX_COMNAM,		'C',1,20,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_LfJst,

"TktType",		"WAYTYP",		0xED,		IX_WAYTYP,		'H',1,2,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"Class",		"CLASS",		0xE,		IX_CLASS,		'H',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"ClassNam",		"CLASSNAM",		0,			IX_CLASSNAM,	'C',1,9,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_RtJst,
"TransType",	"TRNTYP",		'7',		IX_TRNTYP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"TktTypNam",	"WAYTKTNAM",	0,			IX_WAYTKTNAM,	'C',4,TKTNAM_SZ,1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_LfJst,
"WalletNo",		"WALLET",		99999999,	IX_WALLET,		'N',4,8,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"TicketNo",		"TICKETNUM",	999999,		IX_TICKETNUM,	'N',4,8,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"PrcGroup",		"PRCGRP",		0,			IX_PRCGRP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkScm,
"Alight Stage",	"ALTNUM",		9999,		IX_ALTNUM,		'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_RtJst,
"RunAlt",		"RUNALT",		9999,		IX_RUNALT,		'N',2,4,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,
"Alight",		"ALTNAM",		0,			IX_ALTNAM,		'C',2,30+3,		0,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng|F_LfJst|F_IsTxt,
"Travel",		"TRAVEL",		255,		IX_TRAVEL,		'N',1,3,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkRng,   // NumStages
"Rev",			"REV",			0xFFFFFFFF,	IX_REV,			'M',4,8,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_Tkx|F_Money|F_RtJst,
"Psg",			"PSG",			0,			IX_PSG,			'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst,
"Pas",			"PAS",			0,			IX_PAS,			'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst,
"Pax",			"PAX",			0xFFFF,		IX_PAX,			'I',2,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_Tkx|F_RtJst,
"SchedMiles",	"ShdMil",		0,			IX_ShdMil,		'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"SchedLive",	"ShdMilLive",	0,			IX_ShdMilLive,	'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"SchedDead",	"ShdMilDead",	0,			IX_ShdMilDead,	'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"AdjustMiles",	"AdjMil",		0,			IX_AdjMil,		'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"GainMiles",	"LOSTPOS",		0,			IX_LOSTPOS,		'I',4,5,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_OkRng|F_RtJst|F_Miles,
"LostMiles",	"LOSTNEG",		0,			IX_LOSTNEG,		'I',4,5,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_OkRng|F_RtJst|F_Miles,
"AdjustLive",	"AdjMilLive",	0,			IX_AdjMilLive,	'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"AdjustDead",	"AdjMilDead",	0,			IX_AdjMilDead,	'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"LostLiveNeg",	"AdjMilLiveNeg",0,			IX_AdjMilLiveNeg,'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,
"LostDeadNeg",	"AdjMilDeadNeg",0,			IX_AdjMilDeadNeg,'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_RtJst|F_Miles,

"LiveTime",		"MinLive",		999,		IX_MinLive,		'I',4,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_Time,		// 200707
"Dead&Layovr",	"MinDeadLay",	999,		IX_MinDeadLay,	'I',4,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkDat|F_Time,		// 200707

"Calc",			"CALC",			0,			IX_CALC,		'I',4,6,		1,0,0,0,0,0,0,0,0,0,0,0,F_RtJst, // User Value
"TimeBand",		"TIMGRP",		0,			IX_TIMGRP,		'N',1,1,		1,0,0,0,0,0,0,0,0,0,0,0,F_OkKey|F_OkScm,
0,				"Grp0",			0,			'0',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_RtJst,
0,				"Grp1",			0,			'1',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_RtJst,
0,				"Grp2",			0,			'2',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_RtJst,
0,				"Grp3",			0,			'3',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_RtJst,
0,				"Grp4",			0,			'4',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,F_RtJst,
0,				"Grp5",			0,			'5',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,0,
0,				"Grp6",			0,			'6',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,0,
0,				"Grp7",			0,			'7',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,0,
0,				"Grp8",			0,			'8',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,0,
0,				"Grp9",			0,			'9',			'N',2,4,		1,0,0,0,0,0,0,0,0,0,0,0,0,
0,				0,				0,			0,				0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0};

// NOTE Board/Alight StageNames are stored with "Position in Running Order" prepended, making them 3 chars LONGER!


SY *sy;


void setup_sy(int init)
{
int i;
SY	*s;
if (!init)
	{
	if (sy)
		for (s=sy;s->typ;s++)	// Scrap any allocated 'field name' pointers, substitution values, txt_ctl entries
			{
			Scrap(s->niam);
			Scrap(s->subst);
			if (s->stc)
				delete s->stc;
			}
	Scrap(sy);
	return;
	}
for (i=0;StdSY[i].typ;i++) {;}				// Count how many field-id's we've got
sy=(SY*)memadup(StdSY,(i+1)*sizeof(SY));	// Allow for the NULL entry at the end
for (s=sy;s->typ;s++)	// Explicitly allocate 'name' pointer so we can release it later
	{
	if (s->niam) s->niam=stradup(s->niam);
	if (s->flag&F_IsTxt) s->stc=new SY_TXT_CTL(s);
	}
}

void change_sy_name(int typ, const char *newname)
{
for (SY *s=sy; s->typ; s++)
	if (s->typ==typ)
		{
		unsigned	len=strlen(newname);
		if (s->niam && strlen(s->niam)>=len)
			strcpy(s->niam,newname);
		else
			strcpy(s->niam=(char*)memrealloc(s->niam,len+1),newname);
		break;
		}
}

// Move 1, 2, or 4 bytes of value 'n' to passed address in strictly 'most significant byte(s) first' sequence
// O/p is then amenable to simple 'byte compare' logic to sort values without worrying about Intel 'backwords storage' format
// If a null address is passed use a local static storage area. Return that (or passed address) as a convenience
char *memmove_rv_sy(char *ad, int n, SY *s)
{
static char static4[4];
if (!ad) ad=static4;
switch (s->len)
	{
	case 1: ad[0]=(char)n;break;
	case 2: ad[0]=(char)(n>>8);ad[1]=(char)n;break;
	default:ad[0]=(char)(n>>24);ad[1]=(char)(n>>16);ad[2]=(char)(n>>8);ad[3]=(char)n;
	}
return(ad);
}


