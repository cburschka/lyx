// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
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

/**
  This class contains all the parameters for this a buffer uses. Some
  work needs to be done on this class to make it nice. Now everything
  is in public.
  */
class BufferParams {
public:
	//@Man: Constructors and Deconstructors
	//@{
	///
	BufferParams();
	//@}

	/// Dummy destructor to shut up gcc
	virtual ~BufferParams() {}
	
	///
	void writeFile(FILE *);


	///
	void useClassDefaults();

	///
	VSpace getDefSkip() const { return defskip; }

	///
	void setDefSkip(VSpace vs) { defskip = vs; }
	
	/** Wether paragraphs are separated by using a indent like in
	  articles or by using a little skip like in letters.
	  */
  	char paragraph_separation; // add approp. signedness
	///
  	InsetQuotes::quote_language quotes_language;
	///
  	InsetQuotes::quote_times quotes_times;
	///
  	string fontsize; 
	///
 	LyXTextClassList::ClassList::size_type textclass;

	/* this are for the PaperLayout */
   ///
  	char papersize; /* the general papersize (papersize2 or paperpackage */ // add approp. signedness
        ///
        char papersize2; /* the selected Geometry papersize */ // add approp. signedness
        ///
        char paperpackage; /* a special paperpackage .sty-file */ // add approp. signedness
        ///
	char orientation; // add approp. signedness
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
	Bullet temp_bullets[4];
	///
	Bullet user_defined_bullets[4];
	///
	void Copy(BufferParams const &p);
	///
	virtual void readPreamble(LyXLex &);
	///
	virtual void readLanguage(LyXLex &);
	///
	virtual void readGraphicsDriver(LyXLex &);
	/// do we allow accents on all chars in this buffer
	bool allowAccents;
	///
        bool use_amsmath;
	/// Time ago we agreed that this was a buffer property [ale990407]
        string parentname;
protected:
private:
	///
	friend class Buffer;
	/** This is the amount of space used for paragraph_separation "skip",
	  and for detached paragraphs in "indented" documents. */
	VSpace defskip;
};

#endif
