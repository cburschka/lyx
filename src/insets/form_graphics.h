/** Header file generated with fdesign on Fri Apr 30 16:14:04 1999.**/

#ifndef FORM_GRAPHICS_H
#define FORM_GRAPHICS_H

/** Callbacks, globals and object handlers **/

extern "C" void GraphicsCB(FL_OBJECT *, long);

struct FD_Graphics {
	FL_FORM * Graphics;
	void * vdata;
	char * cdata;
	long  ldata;
	FL_OBJECT * graphicsFile;
	FL_OBJECT * Browse;
	FL_OBJECT * Width;
	FL_OBJECT * Height;
	FL_OBJECT * ApplyBtn;
	FL_OBJECT * OkBtn;
	FL_OBJECT * CancelBtn;
	FL_OBJECT * Angle;
	FL_OBJECT * HeightLabel;
	FL_OBJECT * WidthLabel;
};

extern FD_Graphics * create_form_Graphics();


#endif
