// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef BUFFERPARAMS_H
#define BUFFERPARAMS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "lyxlex.h"
#include "vspace.h"
#include "Spacing.h"
#include "Bullet.h"
#include "insets/insetquotes.h"
#include "layout.h"
#include "support/block.h"


struct Language;

/** Buffer parameters.
   This class contains all the parameters for this a buffer uses. Some
   work needs to be done on this class to make it nice. Now everything
   is in public.
*/
class BufferParams {
public:
	///
	enum PAPER_SIZE {
		///
		PAPER_DEFAULT,
		///
		PAPER_USLETTER,
		///
		PAPER_LEGALPAPER,
		///
		PAPER_EXECUTIVEPAPER,
		///
		PAPER_A3PAPER,
		///
		PAPER_A4PAPER,
		///
		PAPER_A5PAPER,
		///
		PAPER_B5PAPER
	};
	///
	enum PAPER_PACKAGES {
		///
		PACKAGE_NONE,
		///
		PACKAGE_A4,
		///
		PACKAGE_A4WIDE,
		///
		PACKAGE_WIDEMARGINSA4
	};
	///
	enum VMARGIN_PAPER_TYPE {
		///
		VM_PAPER_DEFAULT,
		///
		VM_PAPER_CUSTOM,
		///
		VM_PAPER_USLETTER,
		///
		VM_PAPER_USLEGAL,
		///
		VM_PAPER_USEXECUTIVE,
		///
		VM_PAPER_A3,
		///
		VM_PAPER_A4,
		///
		VM_PAPER_A5,
		///
		VM_PAPER_B3,
		///
		VM_PAPER_B4,
		///
		VM_PAPER_B5
	};
	///
	enum PARSEP {
		///
		PARSEP_INDENT,
		///
		PARSEP_SKIP
	};
	///
	enum PAPER_ORIENTATION {
		///
		ORIENTATION_PORTRAIT,
		///
		ORIENTATION_LANDSCAPE
	};
	///
	BufferParams();

	///
	void writeFile(std::ostream &) const;

	///
	void useClassDefaults();

	///
	VSpace const & getDefSkip() const { return defskip; }

	///
	void setDefSkip(VSpace const & vs) { defskip = vs; }

	/** Wether paragraphs are separated by using a indent like in
	  articles or by using a little skip like in letters.
	  */
  	PARSEP paragraph_separation;
	///
  	InsetQuotes::quote_language quotes_language;
	///
  	InsetQuotes::quote_times quotes_times;
	///
  	string fontsize; 
	///
 	LyXTextClassList::size_type textclass;

	/* this are for the PaperLayout */
	/// the general papersize (papersize2 or paperpackage
  	char papersize; // add apprip. signedness 
        ///  the selected Geometry papersize
        char papersize2; // add approp. signedness
        /// a special paperpackage .sty-file
        char paperpackage; // add approp. signedness
        ///
	PAPER_ORIENTATION orientation; // add approp. signedness
	///
        bool use_geometry;
        ///
        string paperwidth;
        ///
        string paperheight;
        ///
        string leftmargin;
        ///
        string topmargin;
        ///
        string rightmargin;
        ///
        string bottommargin;
        ///
        string headheight;
        ///
        string headsep;
        ///
        string footskip;
        
        /* some LaTeX options */
	/// The graphics driver
	string graphicsDriver;
	///
	string fonts;
	///
	Spacing spacing;
	///
	int secnumdepth;
	///
	int tocdepth;
	///
	string language;
	///
	Language const * language_info;
	///
	string inputenc;
	///
	string preamble;
	///
	string options;
	///
	string float_placement;
	///
	unsigned int columns;
	///
	LyXTextClass::PageSides sides;
	///
	string pagestyle;
	///
	block<Bullet, 4> temp_bullets;
	///
	block<Bullet, 4> user_defined_bullets;
	///
	void readPreamble(LyXLex &);
	///
	void readLanguage(LyXLex &);
	///
	void readGraphicsDriver(LyXLex &);
	///
        bool use_amsmath;
	/// Time ago we agreed that this was a buffer property [ale990407]
        string parentname;
private:
	///
	friend class Buffer;
	/** This is the amount of space used for paragraph_separation "skip",
	  and for detached paragraphs in "indented" documents.
	*/
	VSpace defskip;
};

#endif
