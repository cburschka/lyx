// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_toc_h_
#define FD_form_toc_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseHideCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_toc {
	~FD_form_toc();

	FL_FORM *form;
	FL_OBJECT *browser;
	FL_OBJECT *type;
};

#endif /* FD_form_toc_h_ */
