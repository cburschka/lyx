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
	FL_OBJECT *button_findnext;
	FL_OBJECT *button_findprev;
	FL_OBJECT *button_replace;
	FL_OBJECT *button_close;
	FL_OBJECT *check_casesensitive;
	FL_OBJECT *check_matchword;
	FL_OBJECT *button_replaceall;
};

#endif /* FD_form_search_h_ */
