// -*- C++ -*-
/**
 * \file insetgraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voﬂ
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
class RenderGraphic;
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
	void metrics(MetricsInfo &, Dimension &) const;
	///
	EDITABLE editable() const;
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex & lex);

	/** returns the number of rows (\n's) of generated tex code.
	 #fragile == true# means, that the inset should take care about
	 fragile commands by adding a #\protect# before.
	 */
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;

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
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void edit(LCursor & cur, bool left);
	///
	void editGraphics(InsetGraphicsParams const &, Buffer const &) const;
protected:
	///
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
private:
	///
	friend class InsetGraphicsMailer;

	/// Read the inset native format
	void readInsetGraphics(LyXLex & lex, std::string const & bufpath);

	/// Get the status message, depends on the image loading status.
	std::string const statusMessage() const;
	/// Create the options for the latex command.
	std::string const createLatexOptions() const;
	/// Convert the file if needed, and return the location of the file.
	std::string const prepareFile(Buffer const & buf, OutputParams const &) const;

	///
	InsetGraphicsParams params_;

	/// holds the entity name that defines the graphics location (SGML).
	std::string const graphic_label;

	/// The thing that actually draws the image on LyX's screen.
	boost::scoped_ptr<RenderGraphic> const graphic_;
};


#include "mailinset.h"

class InsetGraphicsMailer : public MailInset {
public:
	///
	InsetGraphicsMailer(InsetGraphics & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const & data,
				  Buffer const & buffer,
				  InsetGraphicsParams &);
	///
	static std::string const params2string(InsetGraphicsParams const &,
					  Buffer const &);
private:
	///
	static std::string const name_;
	///
	InsetGraphics & inset_;
};

#endif
