// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_tabbed_thesaurus_h_
#define FD_form_tabbed_thesaurus_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_tabbed_thesaurus {
	~FD_form_tabbed_thesaurus();

	FL_FORM *form;
	FL_OBJECT *tabbed_folder;
	FL_OBJECT *button_replace;
	FL_OBJECT *button_close;
	FL_OBJECT *input_entry;
	FL_OBJECT *input_replace;
};
struct FD_form_noun {
	~FD_form_noun();

	FL_FORM *form;
	FL_OBJECT *browser_noun;
};
struct FD_form_verb {
	~FD_form_verb();

	FL_FORM *form;
	FL_OBJECT *browser_verb;
};
struct FD_form_adjective {
	~FD_form_adjective();

	FL_FORM *form;
	FL_OBJECT *browser_adjective;
};
struct FD_form_adverb {
	~FD_form_adverb();

	FL_FORM *form;
	FL_OBJECT *browser_adverb;
};
struct FD_form_other {
	~FD_form_other();

	FL_FORM *form;
	FL_OBJECT *browser_other;
};

#endif /* FD_form_tabbed_thesaurus_h_ */
