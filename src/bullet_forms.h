/** Header file generated with fdesign on Mon Apr 12 19:09:41 1999.**/

#ifndef FD_form_bullet_h_
#define FD_form_bullet_h_

/** Callbacks, globals and object handlers **/
extern "C" void ChoiceBulletSizeCB(FL_OBJECT *, long);
extern "C" void BulletOKCB(FL_OBJECT *, long);
extern "C" void BulletApplyCB(FL_OBJECT *, long);
extern "C" void BulletCancelCB(FL_OBJECT *, long);
extern "C" void InputBulletLaTeXCB(FL_OBJECT *, long);
extern "C" void BulletDepthCB(FL_OBJECT *, long);
extern "C" void BulletPanelCB(FL_OBJECT *, long);
extern "C" void BulletBMTableCB(FL_OBJECT *, long);


/**** Additional routines ****/

extern void bulletForm();
extern bool updateBulletForm(); 

/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_bullet;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *bmtable_bullet_panel;
	FL_OBJECT *choice_bullet_size;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *input_bullet_latex;
	FL_OBJECT *radio_bullet_depth;
	FL_OBJECT *radio_bullet_depth_1;
	FL_OBJECT *radio_bullet_depth_2;
	FL_OBJECT *radio_bullet_depth_3;
	FL_OBJECT *radio_bullet_depth_4;
	FL_OBJECT *radio_bullet_panel;
	FL_OBJECT *radio_bullet_panel_standard;
	FL_OBJECT *radio_bullet_panel_maths;
	FL_OBJECT *radio_bullet_panel_ding2;
	FL_OBJECT *radio_bullet_panel_ding3;
	FL_OBJECT *radio_bullet_panel_ding4;
	FL_OBJECT *radio_bullet_panel_ding1;
} FD_form_bullet;

extern FD_form_bullet * create_form_form_bullet(void);

#endif /* FD_form_bullet_h_ */
