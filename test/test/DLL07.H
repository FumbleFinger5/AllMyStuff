//////////////////////////////////////////////////////////////////////////////////////
// Declarations for TICKET ARCHIVE MANAGEMENT functions used by VB / C++ INTERFACE  //
//////////////////////////////////////////////////////////////////////////////////////
// INT Functions return 0 on success, error code on failure

#include "dll.h"	// Include "common" interface function declarations,

int __stdcall DllInit(LPSTR lpArcPath);

int __stdcall DllListDiscs(char *buffer);	// return all disc_id's in current archive


int __stdcall DllOpenDisc(char *disc_id);	// disc_id = yyyy-mm-dd hh:mm.ss

int __stdcall DllPutModuleEx(char *hexdata, char *module_filename, char *act_bd, char *update_datetime);
// data = the binary image of the module to be stored
// module_filename = 7-digits, possibly followed by ".2" - ".99"
// act_bd = List of Activity Dates in this module, each format "ddmmyy\t", whole list terminated by \r\0
// update_datetime = OS DateTimeStamp of raw module file, format yyyy-mm-dd hh:mm.ss

int __stdcall DllCloseDisc(void);


// Recursively check TI.INI files in passed path and all subdirectories
// return a full list of all [param_names] found
// If 'find' is non-blank, return the PATH of each TI.INI file in which an entry for 'find' was found
int __stdcall DllCheckTI(char *path, char *vb, char *find);

// Recursively check archives in passed path and all subdirectories
// Log to c:\sjh_err.log any that have duff ctl_btr counts
int __stdcall DllCheckKC(char *path);


// Return all records from Dos XTRACT.DBF (repeat call until buffer returned empty)
int __stdcall DllGetXtract(char *vbuf);

