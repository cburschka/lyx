/** Header file generated with fdesign on Mon Jul 17 21:08:23 2000.**/

#ifndef FD_LaTeXLog_h_
#define FD_LaTeXLog_h_

/** Callbacks, globals and object handlers **/
extern "C" void LatexLogClose(FL_OBJECT *, long);
extern "C" void LatexLogUpdate(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *LaTeXLog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser_latexlog;
} FD_LaTeXLog;

extern FD_LaTeXLog * create_form_LaTeXLog(void);

#endif /* FD_LaTeXLog_h_ */
