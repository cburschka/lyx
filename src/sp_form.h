/** Header file generated with fdesign on Mon Apr 12 19:09:43 1999.**/

#ifndef FD_form_spell_options_h_
#define FD_form_spell_options_h_

/** Callbacks, globals and object handlers **/
extern "C" void SpellOptionsOKCB(FL_OBJECT *, long);
extern "C" void SpellOptionsCancelCB(FL_OBJECT *, long);
extern "C" void SpellOptionsApplyCB(FL_OBJECT *, long);



/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_spell_options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *lang_buts;
	FL_OBJECT *buflang;
	FL_OBJECT *altlang;
	FL_OBJECT *altlang_input;
	FL_OBJECT *compounds;
	FL_OBJECT *inpenc;
	FL_OBJECT *ok;
	FL_OBJECT *perdict;
	FL_OBJECT *esc_chars;
	FL_OBJECT *perdict_input;
	FL_OBJECT *esc_chars_input;
} FD_form_spell_options;

extern FD_form_spell_options * create_form_form_spell_options(void);
typedef struct {
	FL_FORM *form_spell_check;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *text;
	FL_OBJECT *input;
	FL_OBJECT *browser;
	FL_OBJECT *options;
	FL_OBJECT *start;
	FL_OBJECT *insert;
	FL_OBJECT *ignore;
	FL_OBJECT *accept;
	FL_OBJECT *stop;
	FL_OBJECT *done;
	FL_OBJECT *replace;
	FL_OBJECT *slider;
} FD_form_spell_check;

extern FD_form_spell_check * create_form_form_spell_check(void);

#endif /* FD_form_spell_options_h_ */
