// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_bibitem_h_
#define FD_form_bibitem_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_bibitem {
	~FD_form_bibitem();

	FL_FORM *form;
	FL_OBJECT *key;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *label;
};

#endif /* FD_form_bibitem_h_ */
