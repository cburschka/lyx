// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 the LyX Team.
 *
 * \author Baruch Even
 * \author Herbert Voss <voss@lyx.org>
 * ====================================================== */

#ifndef INSET_GRAPHICS_H
#define INSET_GRAPHICS_H


#ifdef __GNUG__
#pragma interface
#endif

#include "insets/inset.h"
#include "graphics/GraphicsTypes.h"
#include "insets/insetgraphicsParams.h"
// We need a signal here to hide an active dialog when we are deleted.
#include "sigc++/signal_system.h"

class Dialogs;
class LaTeXFeatures;

///
class InsetGraphics : public Inset, public SigC::Object {
public:
	///
	InsetGraphics();
	///
	InsetGraphics(InsetGraphics const &, string const & filepath,
		      bool same_id = false);
	///
	~InsetGraphics();
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	EDITABLE editable() const;
	///
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex & lex);

	/** returns the number of rows (\n's) of generated tex code.
	 #fragile == true# means, that the inset should take care about
	 fragile commands by adding a #\protect# before.
	 */
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &) const;

	/** Tell LyX what the latex features you need i.e. what latex packages
	    you need to be included.
	 */
	void validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	Inset::Code lyxCode() const { return Inset::GRAPHICS_CODE; }

	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;

	/** Set the inset parameters, used by the GUIndependent dialog.
	    Return true of new params are different from what was so far.
	*/
	bool setParams(InsetGraphicsParams const & params,
		       string const & filepath);

	/// Get the inset parameters, used by the GUIndependent dialog.
	InsetGraphicsParams const & params() const;

	/** This signal is connected by our dialog and called when the inset
	    is deleted.
	*/
	SigC::Signal0<void> hideDialog;

private:
	/// Set the cached variables
	void setCache() const;
	/// Is the image ready to draw, or should we display a message instead?
	bool drawImage() const;

	/// Read the inset native format
	void readInsetGraphics(LyXLex & lex);
	/// Read the FigInset file format
	void readFigInset(LyXLex & lex);

	/// Update the inset after parameter change.
	void updateInset(string const & filepath) const;
	/// Get the status message, depends on the image loading status.
	string const statusMessage() const;
	/// Create the options for the latex command.
	string const createLatexOptions() const;
	/// Convert the file if needed, and return the location of the file.
	string const prepareFile(Buffer const * buf) const;

	///
	InsetGraphicsParams params_;

	/// holds the entity name that defines the graphics location (SGML).
	string const graphic_label;

	/// The cached variables
	mutable grfx::ImageStatus cached_status_;
	///
	mutable grfx::ImagePtr cached_image_;
	///
	mutable bool cache_filled_;
	///
	mutable int old_asc;
};

#endif
