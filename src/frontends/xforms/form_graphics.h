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
	FL_OBJECT *input_height;
	FL_OBJECT *radio_button_group_width;
	FL_OBJECT *radio_width_percent_page;
	FL_OBJECT *radio_width_default;
	FL_OBJECT *radio_width_cm;
	FL_OBJECT *radio_width_inch;
	FL_OBJECT *radio_width_percent_column;
	FL_OBJECT *radio_button_group_height;
	FL_OBJECT *radio_height_percent_page;
	FL_OBJECT *radio_height_inch;
	FL_OBJECT *radio_height_cm;
	FL_OBJECT *radio_height_default;
	FL_OBJECT *radio_button_group_display;
	FL_OBJECT *radio_display_monochrome;
	FL_OBJECT *radio_display_grayscale;
	FL_OBJECT *radio_display_color;
	FL_OBJECT *radio_no_display;
	FL_OBJECT *input_rotate_angle;
	FL_OBJECT *input_subcaption;
	FL_OBJECT *check_subcaption;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_graphics_h_ */
