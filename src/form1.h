/** Header file generated with fdesign **/

#ifndef FD_KeyMap_h_
#define FD_KeyMap_h_

/** Callbacks, globals and object handlers **/

extern "C" void GraphicsCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *KeyMap;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *AcceptChset;
	FL_OBJECT *ChsetErr;
	FL_OBJECT *KeymapErr;
	FL_OBJECT *Charset;
	FL_OBJECT *Accept;
	FL_OBJECT *OtherKeymap;
	FL_OBJECT *OtherKeymap2;
	FL_OBJECT *KeyMapOn;
	FL_OBJECT *KeyOnBtn;
	FL_OBJECT *KeyOffBtn;
	FL_OBJECT *KeyOnBtn2;
} FD_KeyMap;

extern FD_KeyMap * create_form_KeyMap(void);
typedef struct {
	FL_FORM *Figure;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *EpsFile;
	FL_OBJECT *Preview;
	FL_OBJECT *Browse;
	FL_OBJECT *Width;
	FL_OBJECT *Height;
	FL_OBJECT *ApplyBtn;
	FL_OBJECT *OkBtn;
	FL_OBJECT *CancelBtn;
	FL_OBJECT *Frame;
	FL_OBJECT *Translations;
	FL_OBJECT *Angle;
	FL_OBJECT *HeightGrp;
	FL_OBJECT *page2;
	FL_OBJECT *Default2;
	FL_OBJECT *cm2;
	FL_OBJECT *in2;
	FL_OBJECT *HeightLabel;
	FL_OBJECT *WidthLabel;
	FL_OBJECT *DisplayGrp;
	FL_OBJECT *Wysiwyg3;
	FL_OBJECT *Wysiwyg0;
	FL_OBJECT *Wysiwyg2;
	FL_OBJECT *Wysiwyg1;
	FL_OBJECT *WidthGrp;
	FL_OBJECT *Default1;
	FL_OBJECT *cm1;
	FL_OBJECT *in1;
	FL_OBJECT *page1;
	FL_OBJECT *column1;
	FL_OBJECT *Subcaption;
	FL_OBJECT *Subfigure;
} FD_Figure;

extern FD_Figure * create_form_Figure(void);
typedef struct {
	FL_FORM *form_search;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *input_search;
	FL_OBJECT *input_replace;
	FL_OBJECT *replace_button;
	FL_OBJECT *btnCaseSensitive;
	FL_OBJECT *btnMatchWord;
	FL_OBJECT *replaceall_button;
} FD_form_search;

#endif /* FD_KeyMap_h_ */
