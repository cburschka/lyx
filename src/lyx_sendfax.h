/** Header file generated with fdesign **/

#ifndef FD_xsendfax_h_
#define FD_xsendfax_h_

/** Callbacks, globals and object handlers **/
extern "C" void FaxSendCB(FL_OBJECT *, long);
extern "C" void FaxCancelCB(FL_OBJECT *, long);
extern "C" void FaxApplyCB(FL_OBJECT *, long);
extern "C" void FaxOpenPhonebookCB(FL_OBJECT *, long);
extern "C" void cb_add_phoneno(FL_OBJECT *, long);
extern "C" void cb_delete_phoneno(FL_OBJECT *, long);
extern "C" void cb_save_phoneno(FL_OBJECT *, long);

extern "C" void cb_select_phoneno(FL_OBJECT *, long);

extern "C" void FaxLogfileCloseCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *xsendfax;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *Input_Phone;
	FL_OBJECT *Input_Name;
	FL_OBJECT *Input_Enterprise;
	FL_OBJECT *Input_Comment;
	FL_OBJECT *Button_Send;
	FL_OBJECT *Button_Cancel;
	FL_OBJECT *Button_Apply;
	FL_OBJECT *Button_SPhone;
	FL_OBJECT *pb_save;
} FD_xsendfax;

extern FD_xsendfax * create_form_xsendfax(void);
typedef struct {
	FL_FORM *phonebook;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser;
} FD_phonebook;

extern FD_phonebook * create_form_phonebook(void);
typedef struct {
	FL_FORM *logfile;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser;
	FL_OBJECT *Button_Close;
} FD_logfile;

extern FD_logfile * create_form_logfile(void);

#endif /* FD_xsendfax_h_ */
