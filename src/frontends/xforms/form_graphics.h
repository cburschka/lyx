// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_graphics_h_
#define FD_form_graphics_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_graphics {
	~FD_form_graphics();

	FL_FORM *form;
	FL_OBJECT *tabfolder;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_close;
	FL_OBJECT *button_apply;
	FL_OBJECT *text_warning;
};
struct FD_form_file {
	~FD_form_file();

	FL_FORM *form;
	FL_OBJECT *input_filename;
	FL_OBJECT *button_browse;
	FL_OBJECT *check_subcaption;
	FL_OBJECT *input_subcaption;
	FL_OBJECT *input_rotate_angle;
	FL_OBJECT *choice_origin;
	FL_OBJECT *check_draft;
	FL_OBJECT *check_rotate;
	FL_OBJECT *check_nounzip;
};
struct FD_form_size {
	~FD_form_size();

	FL_FORM *form;
	FL_OBJECT *radio_asis;
	FL_OBJECT *radio_scale;
	FL_OBJECT *radio_wh;
	FL_OBJECT *input_scale;
	FL_OBJECT *input_width;
	FL_OBJECT *choice_width;
	FL_OBJECT *input_height;
	FL_OBJECT *choice_height;
	FL_OBJECT *check_aspectratio;
	FL_OBJECT *button_lyx_values;
};
struct FD_form_bbox {
	~FD_form_bbox();

	FL_FORM *form;
	FL_OBJECT *input_bb_x1;
	FL_OBJECT *input_bb_y1;
	FL_OBJECT *input_bb_x0;
	FL_OBJECT *input_bb_y0;
	FL_OBJECT *choice_bb_units;
	FL_OBJECT *button_getBB;
	FL_OBJECT *check_clip;
};
struct FD_form_special {
	~FD_form_special();

	FL_FORM *form;
	FL_OBJECT *input_special;
};
struct FD_form_lyxview {
	~FD_form_lyxview();

	FL_FORM *form;
	FL_OBJECT *input_lyxscale;
	FL_OBJECT *radio_lyxasis;
	FL_OBJECT *radio_lyxscale;
	FL_OBJECT *radio_lyxwh;
	FL_OBJECT *radio_pref;
	FL_OBJECT *radio_mono;
	FL_OBJECT *radio_gray;
	FL_OBJECT *radio_color;
	FL_OBJECT *radio_nodisplay;
	FL_OBJECT *input_lyxwidth;
	FL_OBJECT *choice_lyxwidth;
	FL_OBJECT *input_lyxheight;
	FL_OBJECT *choice_lyxheight;
	FL_OBJECT *button_latex_values;
};

#endif /* FD_form_graphics_h_ */
