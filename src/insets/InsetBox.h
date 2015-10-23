// -*- C++ -*-
/**
 * \file InsetBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBOX_H
#define INSETBOX_H

#include "InsetCollapsable.h"
#include "Length.h"


namespace lyx {

class InsetBoxParams
{
public:
	///
	InsetBoxParams(std::string const &);
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);

	///
	std::string type;
	/// Is there a parbox?
	bool use_parbox;
	/// Is there a makebox?
	bool use_makebox;
	/// Do we have an inner parbox or minipage to format paragraphs to
	/// columnwidth?
	bool inner_box;
	///
	Length width;
	/// "special" widths, see usrguide.dvi §3.5
	std::string special;
	///
	char pos;
	///
	char hor_pos;
	///
	char inner_pos;
	///
	Length height;
	///
	std::string height_special;
	///
	Length thickness;
	///
	Length separation;
	///
	Length shadowsize;
	///
	std::string framecolor;
	///
	std::string backgroundcolor;
};



///////////////////////////////////////////////////////////////////////
//
// The fbox/fancybox inset
//
///////////////////////////////////////////////////////////////////////


class InsetBox : public InsetCollapsable
{
public:
	///
	enum BoxType {
		Frameless,
		Boxed,
		Framed,
		ovalbox,
		Ovalbox,
		Shadowbox,
		Shaded,
		Doublebox
	};
	///
	InsetBox(Buffer *, std::string const &);

	///
	static std::string params2string(InsetBoxParams const &);
	///
	static void string2params(std::string const &, InsetBoxParams &);
	///
	InsetBoxParams const & params() const { return params_; }

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const { return BOX_CODE; }
	///
	docstring layoutName() const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	DisplayType display() const { return Inline; }
	///
	ColorCode backgroundColor(PainterInfo const &) const;
	///
	LyXAlignment contentAlignment() const;
	///
	bool allowParagraphCustomization(idx_type = 0) const { return !forcePlainLayout(); }
	///
	bool allowMultiPar() const;
	///
	bool forcePlainLayout(idx_type = 0) const;
	///
	bool neverIndent() const { return true; }
	///
	bool inheritFont() const { return false; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	bool hasFixedWidth() const;
	///
	std::string contextMenuName() const;
	//@}

	/// \name Public functions inherited from InsetCollapsable class
	//@{
	///
	void setButtonLabel();
	//@}

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetBox(*this); }
	//@}

private:
	/// used by the constructors
	void init();

	///
	friend class InsetBoxParams;
	InsetBoxParams params_;
};

} // namespace lyx

#endif // INSETBOX_H
