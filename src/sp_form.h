/** Header file generated with fdesign **/

#ifndef FD_form_spell_check_h_
#define FD_form_spell_check_h_

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

#endif /* FD_form_spell_check_h_ */
