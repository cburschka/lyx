// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1999 the LyX Team.
*
*======================================================*/

#ifndef LATEXFEATURES_H
#define LATEXFEATURES_H


#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
using std::vector;

#include "LString.h"

class BufferParams; 
class LyXTextClass;


/** The packages and commands that a buffer needs. This struct
  contains an entry for each of the latex packages and
  commands that a buffer might need. This struct is supposed to be
  extended as the need arises. Remember to update the validate function
  in buffer.C and paragraph.C when you do so. */
struct LaTeXFeatures {
	///
	LaTeXFeatures(int n) ;
	/// The packaes needed by the document
	string getPackages(BufferParams const &params);
	/// The macros definitions needed by the document
	string getMacros(BufferParams const &params);
	/// The definitions needed by the document's textclass
	string getTClassPreamble(BufferParams const &params);

	///
	void showStruct(BufferParams &params);

	//@Man: Packages
	//@{
	///
	bool color;     // color.sty
	///
	bool graphics;  // graphics.sty
	///
	bool setspace;  // setspace.sty
	///
	bool makeidx;   // makeind.sty
	///
	bool verbatim;  // verbatim.sty
	///
	bool longtable; // longtable.sty
	///
	bool algorithm; // algorithm.sty
	///
	bool rotating;  // rotating.sty
	///
	bool amssymb;   // amssymb.sty
	///
	bool latexsym;   // latexsym.sty
	///
	bool pifont;    // pifont.sty
	///
	bool subfigure;	// subfigure.sty
	///
	bool floatflt;	// floatflt.sty
	///
	bool url;       // url.sty
	//@}

	
	//@Man: Commands
	//@{
	///
	bool lyx;
	///
	bool lyxline;
	/// \noun
	bool noun;
	/// \lyxarrow
	bool lyxarrow;
	//@}
	
	//@Man: Quotes
	//@{
	///
	bool quotesinglbase;
	///
	bool quotedblbase;
	///
	bool guilsinglleft;
	///
	bool guilsinglright;
	///
	bool guillemotleft;
	///
	bool guillemotright;
	//@}
	
	//@Man: Math mode
	//@{
	///
	bool amsstyle;
	///
	bool boldsymbol;
	///
	bool binom;
	//@}
	
	//@Man: Layouts
	//@{
	vector<bool> layout;
	//@}
	
	//@Man: Special features
	//@{
	bool LyXParagraphIndent;
	///
	bool NeedLyXFootnoteCode;
	///
	bool NeedLyXMinipageIndent;
	//@}
};

#endif
