/** Header file generated with fdesign **/

#ifndef FD_panel_h_
#define FD_panel_h_

/** Callbacks, globals and object handlers **/
extern "C" void button_cb(FL_OBJECT *, long);

extern "C" void delim_cb(FL_OBJECT *, long);

extern "C" void matrix_cb(FL_OBJECT *, long);

extern "C" void deco_cb(FL_OBJECT *, long);

extern "C" void space_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *panel;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *func_browse;
	FL_OBJECT *greek;
	FL_OBJECT *arrow;
	FL_OBJECT *boperator;
	FL_OBJECT *brelats;
	FL_OBJECT *varsize;
	FL_OBJECT *misc;
	FL_OBJECT *equation;
	FL_OBJECT *sqrt;
	FL_OBJECT *frac;
	FL_OBJECT *delim;
	FL_OBJECT *matrix;
	FL_OBJECT *deco;
	FL_OBJECT *space;
} FD_panel;

extern FD_panel * create_form_panel(void);
typedef struct {
	FL_FORM *delim;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *menu;
	FL_OBJECT *lado;
	FL_OBJECT *right;
	FL_OBJECT *left;
	FL_OBJECT *pix;
} FD_delim;

extern FD_delim * create_form_delim(void);
typedef struct {
	FL_FORM *matrix;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *rows;
	FL_OBJECT *columns;
	FL_OBJECT *valign;
	FL_OBJECT *halign;
} FD_matrix;

extern FD_matrix * create_form_matrix(void);
typedef struct {
	FL_FORM *deco;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *menu;
} FD_deco;

extern FD_deco * create_form_deco(void);
typedef struct {
	FL_FORM *space;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *spaces;
} FD_space;

extern FD_space * create_form_space(void);

#endif /* FD_panel_h_ */
