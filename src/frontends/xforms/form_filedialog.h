// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_filedialog_h_
#define FD_form_filedialog_h_

/** Callbacks, globals and object handlers **/


/**** Forms and Objects ****/
struct FD_form_filedialog {
	~FD_form_filedialog();

	FL_FORM *form;
	FL_OBJECT *FileInfo;
	FL_OBJECT *DirBox;
	FL_OBJECT *PatBox;
	FL_OBJECT *List;
	FL_OBJECT *Filename;
	FL_OBJECT *Rescan;
	FL_OBJECT *Home;
	FL_OBJECT *User1;
	FL_OBJECT *User2;
	FL_OBJECT *Ready;
	FL_OBJECT *Cancel;
	FL_OBJECT *timer;
};

#endif /* FD_form_filedialog_h_ */
