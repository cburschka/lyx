// -*- C++ -*-
/**
 * \file insetcharstyle.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCHARSTYLE_H
#define INSETCHARSTYLE_H

#include "insetcollapsable.h"
#include "lyxtextclass.h"


struct InsetCharStyleParams {
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	std::string type;
	///
	std::string latextype;
	///
	std::string latexname;
	///
	std::string latexparam;
	///
	LyXFont font;
	///
	LyXFont labelfont;
};


/** The CharStyle inset, also XML short element

*/
class InsetCharStyle : public InsetCollapsable {
public:
	///
	InsetCharStyle(BufferParams const &, CharStyles::iterator);
	/// Copy constructor
	InsetCharStyle(InsetCharStyle const &);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	std::string const editMessage() const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::CHARSTYLE_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo &, int, int) const;
	///
	void getDrawFont(LyXFont &) const;
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;

	///
	InsetCharStyleParams const & params() const { return params_; }

protected:
	///
	virtual void priv_dispatch(LCursor & cur, FuncRequest & cmd);

private:
	///
	friend class InsetCharStyleParams;

	/// used by the constructors
	void init();
	///
	InsetCharStyleParams params_;
	///
	bool has_label_;
};

#endif
