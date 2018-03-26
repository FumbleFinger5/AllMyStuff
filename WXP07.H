#include "xlib.h"
#include "dll07.h"
#include "xsq.h"
#include "ssn.h"
#include "DGRP.h"
//#include "access.H"

struct CMFC {short fct, dct, dvd; long sz; char *mn;};

class SMDB {
public:
SMDB(char *fn);						// Constructor used by app progs
~SMDB() {/*Scrap(sn);*/ dbsafeclose(db);};
void	upd(int slot, DYNAG *em);
void	zap(int slot);
void	test(void);
void	test1(void);
void	test2(void);
void	test3(void);

private:
int		ct;
HDL		db,em_btr;
void	db_open(char *fn);
RHDL	rhdl;
struct	{char ver, fill[3]; RHDL em_rhdl; char fill2[24];} hdr;
};

