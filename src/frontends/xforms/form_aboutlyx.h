// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_aboutlyx_h_
#define FD_form_aboutlyx_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);



/**** Forms and Objects ****/
struct FD_form_aboutlyx {
	~FD_form_aboutlyx();

	FL_FORM *form;
	FL_OBJECT *close;
	FL_OBJECT *tabbed_folder;
};
struct FD_form_tab_version {
	~FD_form_tab_version();

	FL_FORM *form;
	FL_OBJECT *text_copyright;
	FL_OBJECT *text_version;
};
struct FD_form_tab_credits {
	~FD_form_tab_credits();

	FL_FORM *form;
	FL_OBJECT *browser_credits;
};
struct FD_form_tab_license {
	~FD_form_tab_license();

	FL_FORM *form;
	FL_OBJECT *text_license;
	FL_OBJECT *text_warranty;
};

#endif /* FD_form_aboutlyx_h_ */
