/** Header file generated with fdesign on Wed Aug  2 13:56:57 2000.**/

#ifndef FD_form_graphics_h_
#define FD_form_graphics_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormGraphicsInputCB(FL_OBJECT *, long);
extern  "C" void C_FormGraphicsBrowseCB(FL_OBJECT *, long);
extern  "C" void C_FormGraphicsOKCB(FL_OBJECT *, long);
extern  "C" void C_FormGraphicsApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormGraphicsCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_graphics {

	FL_FORM *form_graphics;
	FL_OBJECT *input_filename;
	FL_OBJECT *button_browse;
	FL_OBJECT *input_width;
	FL_OBJECT *input_height;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
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
	FL_OBJECT *check_inline;
	FL_OBJECT *input_subcaption;
	FL_OBJECT *check_subcaption;
};

#endif /* FD_form_graphics_h_ */
