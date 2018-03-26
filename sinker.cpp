#include <stdio.h>
#include "WXP07.H"

int chkisdir(char *pth, char *p)	// 'p' is the OUTPUT field we normalise ('pth' is input only)
{
drfulldir(p, pth);
if (!drisdir(p)) return(NO);
strcat(p,"*.*");
return(YES);
}

int sinker(char *pth1, char *pth2)
{
char p1[FNAMSIZ], p2[FNAMSIZ];
int er;
if ((er=chkisdir(pth1,p1))!=0
	|| (er=chkisdir(pth2,p2))!=0)
	return(er);
// we now know p1 and p2 are both valid directories
if (*pth1==99)
	*pth1=98;
return(0);
}
