// -*- C++ -*-
/*
  figinset.h - Figure inset header - part of LyX project
  (C)1996 by Ivan Schreter
  */

#ifndef _FIGINSET_H
#define _FIGINSET_H

#include "form1.h"
#include "buffer.h"
#include "LString.h"
#include "LaTeXFeatures.h"

/* the rest is figure stuff */

struct Figref;

///
class InsetFig: public Inset {
public:
	///
	InsetFig(int tmpx, int tmpy, Buffer *);
	///
	~InsetFig();
	///
	int Ascent(LyXFont const &font) const;
	///
	int Descent(LyXFont const &font) const;
	///
	int Width(LyXFont const &font) const;
	///
	void Draw(LyXFont font, LyXScreen &scr, int baseline, float &x);
	///
	void Write(FILE *file);
	///
	void Read(LyXLex &lex);
	///
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(LString &file, signed char fragile);
	///
	int Linuxdoc(LString &file);
	///
	int DocBook(LString &file);
	/// Updates needed features for this inset.
	void Validate(LaTeXFeatures &features) const;

	/// what appears in the minibuffer when opening
	char const* EditMessage() {return "Opened figure";}
	///
	void Edit(int, int);
	///
	unsigned char Editable() const;
	///
	bool Deletable() const;
	///
	Inset::Code LyxCode() const;
	///
	Inset* Clone();
	///
	void CallbackFig(long arg);
	///
	void Preview(char const *p);
	/// browse for file
	void BrowseFile();

	/// form for user input
	FD_Figure *form;
	/// width and height in pixels on screen
	int wid, hgh;
	/// width and height in postscript units (1/72 inch)
	int pswid, pshgh;
	/// width of raw figure w/o rotation
	int raw_wid;
	/// heigt of raw figure w/o rotation
	int raw_hgh;
	/// x and y coordinate in ps units
	int psx, psy;

	/// .eps file name
	LString fname;
	/// changed filename -> for recompute
	bool changedfname;

	///
	enum HWTYPE {
		///
		DEF,
		///
		CM,
		///
		IN,
		///
		PER_PAGE,
		///
		PER_COL
	};
	
	/// width and height types: 0-default, 1-cm, 2-in, 3-%of page
        /// For width, there is also: 4-% of col
	HWTYPE wtype;
	///
	HWTYPE htype;

	/// temporary w and h type
	HWTYPE twtype;
	///
	HWTYPE thtype;

	/// width and height
	float xwid, xhgh;

	/// rotation angle
	float angle;
	
	/// graphics command, latex version
	LString cmd;
	
	/// Caption for subfigure package
	LString subcaption;

	/// various flags
	int flags;
	bool subfigure : 1;
	/// figure reference
	Figref *figure;
	/// temporary flags
	int pflags;
	bool psubfigure : 1;
private:

	///
	Buffer *owner;
	/// restore values on the form
	void RestoreForm();
	/// recompute screen params
	void Recompute();
	/// regenerate \includegraphics{} command
	void Regenerate();
	/// regenerate \inlcudegraphics{} command in temporary buffer
	void TempRegenerate();
	/// get sizes from .eps file
	void GetPSSizes();
};


///
struct figdata {
	/// bitmap data
	Pixmap bitmap;
	/// reference count for this bitmap
	int ref;
        /// rotation angle
        float angle;
        /// width of raw figure w/o rotation
	int raw_wid;
        /// heigt of raw figure w/o rotation
	int raw_hgh;
	/// width and height on screen
	int wid, hgh;
	/// pointer to file name
	LString fname;
	/// type; 0-none, 1-B/W, 2-Grayscale, 3-Color
	char flags;
	/// reading request is pending on this figure
	bool reading;
	/// could not read it, because it's broken
	bool broken;
	/// when gs was killed, this says that image is ok
	bool gsdone;
	/// gs pid for reading
	int gspid;
};


///
struct Figref {
	/// figure data (image)
	figdata *data;
	/// inset of this figure
	InsetFig *inset;
};

#endif
