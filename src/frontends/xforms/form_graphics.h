// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_graphics_h_
#define FD_form_graphics_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_graphics {
	~FD_form_graphics();

	FL_FORM *form;
	FL_OBJECT *tabFolder;
	FL_OBJECT *button_help;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *text_warning;
};
struct FD_form_file {
	~FD_form_file();

	FL_FORM *form;
	FL_OBJECT *input_filename;
	FL_OBJECT *button_browse;
	FL_OBJECT *choice_display;
	FL_OBJECT *input_lyxwidth;
	FL_OBJECT *choice_width_lyxwidth;
	FL_OBJECT *input_lyxheight;
	FL_OBJECT *choice_width_lyxheight;
	FL_OBJECT *check_subcaption;
	FL_OBJECT *input_subcaption;
};
struct FD_form_size {
	~FD_form_size();

	FL_FORM *form;
	FL_OBJECT *radio_size;
	FL_OBJECT *button_default;
	FL_OBJECT *button_wh;
	FL_OBJECT *button_scale;
	FL_OBJECT *input_width;
	FL_OBJECT *choice_width_units;
	FL_OBJECT *input_height;
	FL_OBJECT *choice_height_units;
	FL_OBJECT *input_scale;
	FL_OBJECT *check_aspectratio;
};
struct FD_form_bbox {
	~FD_form_bbox();

	FL_FORM *form;
	FL_OBJECT *input_bb_x0;
	FL_OBJECT *choice_bb_x0;
	FL_OBJECT *input_bb_x1;
	FL_OBJECT *choice_bb_x1;
	FL_OBJECT *input_bb_y0;
	FL_OBJECT *choice_bb_y0;
	FL_OBJECT *input_bb_y1;
	FL_OBJECT *choice_bb_y1;
	FL_OBJECT *button_clip;
	FL_OBJECT *button_draft;
	FL_OBJECT *button_getBB;
};
struct FD_form_special {
	~FD_form_special();

	FL_FORM *form;
	FL_OBJECT *input_rotate_angle;
	FL_OBJECT *input_special;
	FL_OBJECT *choice_origin;
};

#endif /* FD_form_graphics_h_ */
