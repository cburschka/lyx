// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_external_h_
#define FD_form_external_h_

/** Callbacks, globals and object handlers **/
extern  "C" void ExternalTemplateCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void ExternalBrowseCB(FL_OBJECT *, long);
extern  "C" void ExternalEditCB(FL_OBJECT *, long);
extern  "C" void ExternalViewCB(FL_OBJECT *, long);
extern  "C" void ExternalUpdateCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_external {
	~FD_form_external();

	FL_FORM *form;
	FL_OBJECT *choice_template;
	FL_OBJECT *input_filename;
	FL_OBJECT *button_filenamebrowse;
	FL_OBJECT *input_parameters;
	FL_OBJECT *button_edit;
	FL_OBJECT *button_view;
	FL_OBJECT *button_update;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *browser_helptext;
};

#endif /* FD_form_external_h_ */
