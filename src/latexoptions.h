/** Header file generated with fdesign on Mon Apr 12 19:09:41 1999.**/

#ifndef FD_LaTeXOptions_h_
#define FD_LaTeXOptions_h_

/** Callbacks, globals and object handlers **/
extern "C" void LaTeXOptionsOK(FL_OBJECT *, long);
extern "C" void LaTeXOptionsApply(FL_OBJECT *, long);
extern "C" void LaTeXOptionsCancel(FL_OBJECT *, long);

extern "C" void LatexLogClose(FL_OBJECT *, long);
extern "C" void LatexLogUpdate(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *LaTeXOptions;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *accents;
} FD_LaTeXOptions;

extern FD_LaTeXOptions * create_form_LaTeXOptions(void);
typedef struct {
	FL_FORM *LaTeXLog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser_latexlog;
} FD_LaTeXLog;

extern FD_LaTeXLog * create_form_LaTeXLog(void);

#endif /* FD_LaTeXOptions_h_ */
