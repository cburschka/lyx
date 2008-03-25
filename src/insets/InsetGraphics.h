// -*- C++ -*-
/**
 * \file InsetGraphics.h
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

#include "Inset.h"
#include "InsetGraphicsParams.h"
#include "MailInset.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>


namespace lyx {

class RenderGraphic;
class LaTeXFeatures;

///
class InsetGraphics : public Inset, public boost::signals::trackable {
public:
	///
	InsetGraphics(Buffer & buf);
	///
	~InsetGraphics();
	///
	void setBuffer(Buffer & buffer);
	///
	bool isLabeled() const { return true; }
	void metrics(MetricsInfo &, Dimension &) const;
	///
	EDITABLE editable() const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);

	/** returns the number of rows (\n's) of generated tex code.
	 #fragile == true# means, that the inset should take care about
	 fragile commands by adding a #\protect# before.
	 */
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;

	/** Tell LyX what the latex features you need i.e. what latex packages
	    you need to be included.
	 */
	void validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	InsetCode lyxCode() const { return GRAPHICS_CODE; }

	/** Set the inset parameters, used by the GUIndependent dialog.
	    Return true of new params are different from what was so far.
	*/
	bool setParams(InsetGraphicsParams const & params);

	/// Get the inset parameters, used by the GUIndependent dialog.
	InsetGraphicsParams const & params() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	void editGraphics(InsetGraphicsParams const &, Buffer const &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	/// all graphics can be embedded
	void registerEmbeddedFiles(EmbeddedFileList &) const;
	///
	void updateEmbeddedFile(EmbeddedFile const &);
	///
	void addToToc(ParConstIterator const &) const;
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;

	/// Force inset into LTR environment if surroundings are RTL?
	virtual bool forceLTR() const { return true; }
protected:
	InsetGraphics(InsetGraphics const &);
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	friend class InsetGraphicsMailer;

	virtual Inset * clone() const;

	/// Get the status message, depends on the image loading status.
	std::string const statusMessage() const;
	/// Create the options for the latex command.
	std::string const createLatexOptions() const;
	/// Create length values for docbook export.
	docstring const toDocbookLength(Length const & len) const;
	/// Create the atributes for docbook export.
	docstring const createDocBookAttributes() const;
	/// Convert the file if needed, and return the location of the file.
	std::string prepareFile(OutputParams const &) const;

	///
	InsetGraphicsParams params_;

	/// holds the entity name that defines the graphics location (SGML).
	docstring const graphic_label;

	/// The thing that actually draws the image on LyX's screen.
	boost::scoped_ptr<RenderGraphic> const graphic_;
};


class InsetGraphicsMailer : public MailInset {
public:
	///
	InsetGraphicsMailer(InsetGraphics & inset);
	///
	virtual Inset & inset() const { return inset_; }
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


} // namespace lyx

#endif
