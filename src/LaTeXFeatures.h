// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 the LyX Team.
*
* ====================================================== */

#ifndef LATEXFEATURES_H
#define LATEXFEATURES_H


#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include <set>

#include "LString.h"

class BufferParams; 
class LyXTextClass;
struct Language;

/** The packages and commands that a buffer needs. This struct
  contains an entry for each of the latex packages and
  commands that a buffer might need. This struct is supposed to be
  extended as the need arises. Remember to update the validate function
  in buffer.C and paragraph.C when you do so. */
struct LaTeXFeatures {
	///
	explicit LaTeXFeatures(int n) ;
	/// The packaes needed by the document
	string getPackages(BufferParams const & params);
	/// The macros definitions needed by the document
	string getMacros(BufferParams const & params);
	/// The definitions needed by the document's textclass
	string getTClassPreamble(BufferParams const & params);

	///
	void showStruct(BufferParams const & params);

	//@Man: Packages
	//@{
	///
	bool color;     // color.sty
#ifdef USE_GRAPHICX
	///
	bool graphicx; // graphicx.sty
#else
	///
	bool graphics;  // graphics.sty
#endif
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
	std::vector<bool> layout;
	//@}
	
	//@Man: Special features
	//@{
	bool LyXParagraphIndent;
	///
	bool NeedLyXFootnoteCode;
	///
	bool NeedLyXMinipageIndent;
	///
	typedef std::set<Language const *> LanguageList;
	///
	LanguageList UsedLanguages;
	//@}
};

#endif
