class TI_VAR {
public:
TI_VAR();				// Create Search engine
~TI_VAR();				// Release
char *get(const char *setting_name, char *returned_value, int option_bitflag);
private:
DYNAG	*pth;
};

extern TI_VAR *tiv;
