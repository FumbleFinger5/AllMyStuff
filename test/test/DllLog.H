struct LogArg {char type, io, *value;};

class DllLog
{
public:
DllLog(char *_func,int *_err,...);
~DllLog();
private:
char* logtimetext(void);
long	then, then_ms, toggle, log_this_api;
char	*func;
int		*err;
DYNAG	*arg;
};

int nolog_this_dll(void);	// exposed to avoid unwanted logging by global_closedown
