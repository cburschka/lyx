// -*- C++ -*-
/*
  figinset.h - Figure inset header - part of LyX project
  Copyright 1996 by Ivan Schreter
  */

#ifndef FIGINSET_H
#define FIGINSET_H

#include "form1.h"
#include "LString.h"
#include "buffer.h"
#include "LaTeXFeatures.h"
#include "insets/lyxinset.h"
#include <sigc++/signal_system.h>
/* the rest is figure stuff */

struct Figref;

///
class InsetFig: public Inset, public SigC::Object {
public:
	///
	InsetFig(int tmpx, int tmpy, Buffer const &);
	///
	~InsetFig();
	///
	int ascent(BufferView *, LyXFont const & font) const;
	///
	int descent(BufferView *, LyXFont const & font) const;
	///
	int width(BufferView *, LyXFont const & font) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_space) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	/// Updates needed features for this inset.
	void Validate(LaTeXFeatures & features) const;

	/// what appears in the minibuffer when opening
	string const EditMessage() const;
	
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const;
	///
	bool Deletable() const;
	///
	Inset::Code LyxCode() const;
	///
	Inset * Clone(Buffer const &) const;
	///
	void CallbackFig(long arg);
	///
	void Preview(string const & p);
	/// browse for file
	void BrowseFile();

	/// form for user input
	FD_Figure * form;
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
	string fname;
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
	mutable string cmd;
	
	/// Caption for subfigure package
	string subcaption;

	/// various flags
	int flags;
	///
	bool subfigure;
	/// figure reference
	Figref * figure;
	/// temporary flags
	int pflags;
	///
	bool psubfigure;
private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped).
	*/
	void redraw();

	///
	Buffer const * owner;
	/// restore values on the form
	void RestoreForm();
	/// recompute screen params
	void Recompute();
	/// regenerate \includegraphics{} command
	void Regenerate() const;
	/// regenerate \inlcudegraphics{} command in temporary buffer
	void TempRegenerate();
	/// get sizes from .eps file
	void GetPSSizes();
	/// Redraw connection.
	SigC::Connection r_;
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
	string fname;
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
	figdata * data;
	/// inset of this figure
	InsetFig * inset;
};

#endif
