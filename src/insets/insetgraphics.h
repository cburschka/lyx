// -*- C++ -*-
/**
 * \file insetgraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_GRAPHICS_H
#define INSET_GRAPHICS_H

#include "inset.h"
#include "insetgraphicsParams.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>


class Dialogs;
class GraphicRenderer;
class LaTeXFeatures;

///
class InsetGraphics : public InsetOld, public boost::signals::trackable {
public:
	///
	InsetGraphics();
	///
	InsetGraphics(InsetGraphics const &);
	///
	~InsetGraphics();
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
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
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;

	/** Tell LyX what the latex features you need i.e. what latex packages
	    you need to be included.
	 */
	void validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	InsetOld::Code lyxCode() const { return InsetOld::GRAPHICS_CODE; }

	///
	virtual std::auto_ptr<InsetBase> clone() const;

	/** Set the inset parameters, used by the GUIndependent dialog.
	    Return true of new params are different from what was so far.
	*/
	bool setParams(InsetGraphicsParams const & params);

	/// Get the inset parameters, used by the GUIndependent dialog.
	InsetGraphicsParams const & params() const;

private:
	///
	friend class InsetGraphicsMailer;

	/** This method is connected to the graphics loader, so we are
	 *  informed when the image has been loaded.
	 */
	void statusChanged();

	/// Read the inset native format
	void readInsetGraphics(LyXLex & lex, string const & bufpath);

	/// Get the status message, depends on the image loading status.
	string const statusMessage() const;
	/// Create the options for the latex command.
	string const createLatexOptions() const;
	/// Convert the file if needed, and return the location of the file.
	string const prepareFile(Buffer const * buf, LatexRunParams const &) const;

	///
	InsetGraphicsParams params_;

	/// holds the entity name that defines the graphics location (SGML).
	string const graphic_label;

	/// The thing that actually draws the image on LyX's screen.
	boost::scoped_ptr<GraphicRenderer> const graphic_;
};


#include "mailinset.h"

class InsetGraphicsMailer : public MailInset {
public:
	///
	InsetGraphicsMailer(InsetGraphics & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string(Buffer const &) const;
	///
	static void string2params(string const & data,
				  Buffer const & buffer,
				  InsetGraphicsParams &);
	///
	static string const params2string(InsetGraphicsParams const &,
					  Buffer const &);
private:
	///
	static string const name_;
	///
	InsetGraphics & inset_;
};

#endif
