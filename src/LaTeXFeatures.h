// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2001 the LyX Team.
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
#include "layout.h"

class BufferParams; 
struct Language;

/** The packages and commands that a buffer needs. This struct
    contains an entry for each of the latex packages and
    commands that a buffer might need. This struct is supposed to be
    extended as the need arises. Remember to update the validate function
    in buffer.C and paragraph.C when you do so.
*/
struct LaTeXFeatures {
	///
	LaTeXFeatures(BufferParams const &, LyXTextClass::size_type n) ;
	/// The packaes needed by the document
	string const getPackages() const;
	/// The macros definitions needed by the document
	string const getMacros() const;
	/// The definitions needed by the document's textclass
	string const getTClassPreamble() const;
	///
	string const getIncludedFiles(string const & fname) const;
	///
	void getFloatDefinitions(std::ostream & os) const;

	///
	void showStruct() const;

	/// Provide a string name-space to the requirements
	void require(string const & name);

	/// Static preamble bits from the external material insets
	string externalPreambles;

	///
	bool array;
	///
	bool color;     // color.sty
	///
	bool graphicx; // graphicx.sty
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
	//bool algorithm; // algorithm.sty
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
	///
	bool varioref;  // varioref.sty
	///
	bool prettyref; // prettyref.sty
	///
	bool chess;	// chess.sty

	///
	bool lyx;
	///
	bool lyxline;
	/// \noun
	bool noun;
	/// \lyxarrow
	bool lyxarrow;

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

	///
	bool amsstyle;
	///
	bool boldsymbol;
	///
	bool binom;
	
	std::vector<bool> layout;

	///
	bool ParagraphIndent;
	///
	bool NeedLyXFootnoteCode;
	///
	bool NeedLyXMinipageIndent;
	///
	typedef std::set<Language const *> LanguageList;
	///
	LanguageList UsedLanguages;
	///
	typedef std::set<string> UsedFloats;
	///
	UsedFloats usedFloats;
	///
	typedef std::map<string , string> FileMap;
	///
	FileMap IncludedFiles;
	///
	BufferParams const & bufferParams() const;
private:
	///
	BufferParams const & params;
};

#endif
