// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_search_h_
#define FD_form_search_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_search {
	~FD_form_search();

	FL_FORM *form;
	FL_OBJECT *input_search;
	FL_OBJECT *input_replace;
	FL_OBJECT *findnext;
	FL_OBJECT *findprev;
	FL_OBJECT *replace;
	FL_OBJECT *button_cancel;
	FL_OBJECT *casesensitive;
	FL_OBJECT *matchword;
	FL_OBJECT *replaceall;
};

#endif /* FD_form_search_h_ */
