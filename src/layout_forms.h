/** Header file generated with fdesign **/

#ifndef FD_form_character_h_
#define FD_form_character_h_

/** Callbacks, globals and object handlers **/
extern "C" void CharacterApplyCB(FL_OBJECT *, long);
extern "C" void CharacterCloseCB(FL_OBJECT *, long);

extern "C" void PreambleOKCB(FL_OBJECT *, long);
extern "C" void PreambleApplyCB(FL_OBJECT *, long);
extern "C" void PreambleCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_character;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *choice_family;
	FL_OBJECT *choice_series;
	FL_OBJECT *choice_shape;
	FL_OBJECT *choice_size;
	FL_OBJECT *choice_bar;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_close;
	FL_OBJECT *choice_color;
	FL_OBJECT *check_toggle_all;
	FL_OBJECT *choice_language;
} FD_form_character;

extern FD_form_character * create_form_form_character(void);
typedef struct {
	FL_FORM *form_preamble;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *input_preamble;
} FD_form_preamble;

extern FD_form_preamble * create_form_form_preamble(void);

#endif /* FD_form_character_h_ */
