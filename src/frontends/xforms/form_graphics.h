// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_graphics_h_
#define FD_form_graphics_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_graphics {
	~FD_form_graphics();

	FL_FORM *form;
	FL_OBJECT *input_filename;
	FL_OBJECT *button_browse;
	FL_OBJECT *input_width;
	FL_OBJECT *choice_width_units;
	FL_OBJECT *input_height;
	FL_OBJECT *choice_height_units;
	FL_OBJECT *input_scale;
	FL_OBJECT *input_rotate_angle;
	FL_OBJECT *check_display;
	FL_OBJECT *check_subcaption;
	FL_OBJECT *input_subcaption;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_graphics_h_ */
