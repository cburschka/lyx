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

#ifndef INSET_GRAPHICS_H
#define INSET_GRAPHICS_H


#ifdef __GNUG__
#pragma interface
#endif 

#include "insets/lyxinset.h"
#include "insets/insetgraphicsParams.h"
#include "graphics/GraphicsCacheItem.h"

#include "LaTeXFeatures.h"

// We need a signal here to hide an active dialog when we are deleted.
#include "sigc++/signal_system.h"
#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
using SigC::slot;
using SigC::Object;
#endif 

class Dialogs;
class LyXImage;

///
#ifdef SIGC_CXX_NAMESPACES
class InsetGraphics : public Inset, public SigC::Object
#else
class InsetGraphics : public Inset, public Object
#endif
{
public:
	///
	InsetGraphics();
	///
	~InsetGraphics();
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	bool display() const { return true; }
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);

	/** returns the number of rows (\n's) of generated tex code.
	 #fragile == true# means, that the inset should take care about
	 fragile commands by adding a #\protect# before.
	 */
	int Latex(Buffer const *, std::ostream &,
	          bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;

	/** Tell LyX what the latex features you need i.e. what latex packages
	    you need to be included.
	 */
	void Validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	Inset::Code LyxCode() const { return Inset::GRAPHICS_CODE; }

	///
	Inset * Clone(Buffer const &) const;

	/** Set the inset parameters, used by the GUIndependent dialog.
	    Return true of new params are different from what was so far.
	*/
	bool setParams(InsetGraphicsParams const & params);

	/// Get the inset parameters, used by the GUIndependent dialog.
	InsetGraphicsParams getParams() const;

	/** This signal is connected by our dialog and called when the inset
	    is deleted.
	*/
	Signal0 <void> hideDialog;

private:
	/// Update the inset after parameter change.
	void updateInset() const;
	/// Get the status message, depends on the image loading status.
	char const * statusMessage() const;
	/// Create the options for the latex command.
	string const createLatexOptions() const;
	/// Convert the file if needed, and return the location of the file.
	string const prepareFile(Buffer const * buf) const;

	/// The graphics cache handle.
	mutable GraphicsCacheItem * cacheHandle;

	/// is the pixmap initialized?
	mutable bool imageLoaded;

	InsetGraphicsParams params;
};

#endif 
