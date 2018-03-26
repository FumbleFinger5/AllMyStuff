/////////////////////////////////////////////////////////////////////////////////////
// Wrapper Function definitions to avoid linking static libs into console harness  //
/////////////////////////////////////////////////////////////////////////////////////
// Functions return 0 on success, error code on failure

#include <windows.h>

typedef	char 	 *HDL;	/* abstract data type handle */
#define NOTFND (-1)
#define FNAMSIZ 256				// Max fully-qualified path is 255 chars + nullbyte

char	*Xvb_field(const char *rec, int n);


int		Xa2i(const char *str, int len);
int		Xbigbuf(char **bigbuf, const char *smallbuf);
int		Xc2bd(const char *bd);
char	*Xdmy_stri(short bd);
int		Xstridxc(char,const char*);
char	*Xstrinsc(char *str,int chr);
char	*Xstrdel(char *str, int ct);
char	*Xstrfmt(char *str, const char *ctl,...);
char	*Xstrend(const char *str);
void	Xstrendfmt(char *s, const char *ctl,...);
void	Xmemchk(char *txt);
char	*Xstrupr(char *s);

int		Xtimer(int tmr);

char	*Xstrins(char *str, const char *ins);
int		Xvb2bd(const char *bd);
HDL		Xflopen(const char *fnam, const char *mode);
void	Xflputln(const void *s, HDL h_fl);
void	Xflclose(HDL h_fl);
int		Xfleof(HDL);
int		Xflget(void* a, int maxbytes, HDL f);
int		Xflgetln(void* a, int maxlen, HDL f);
char	*Xfload(char *fn, char *buf);
void	Xsjhlog(char *str);

void	__stdcall Xgetcwd(char *cwd);
char	*Xvb_item(const char *str, int n, char *dst);
char	*Xfieldname(int typ, char *nam);
void	__stdcall Xchdir(char *pth);

void	__stdcall Xwait(const char *txt);
int		__stdcall Xkbstr(char *ip, int len, int flag);

void	__stdcall XStatusCallback(int done, int total);
int		__stdcall XCancelCallback(void);



// *******   Maximum size of text buffers passed to DLL functions   ********
#define VBUF_64K 65000

// buffer size for DllGetJnyTEXT()
#define VBUF_JNYTXT_SZ 128

int Sjh_SetSessionDateRange(void);
