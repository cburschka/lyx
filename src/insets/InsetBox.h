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

#include "InsetCollapsible.h"

#include "support/Length.h"


namespace lyx {

class InsetBoxParams
{
public:
	///
	explicit InsetBoxParams(std::string const &);
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


class InsetBox : public InsetCollapsible
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
	InsetCode lyxCode() const override { return BOX_CODE; }
	///
	docstring layoutName() const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	ColorCode backgroundColor(PainterInfo const &) const override;
	///
	LyXAlignment contentAlignment() const override;
	///
	bool allowParagraphCustomization(idx_type = 0) const override { return !forcePlainLayout(); }
	///
	bool allowMultiPar() const override;
	///
	bool forcePlainLayout(idx_type = 0) const override;
	///
	bool needsCProtection(bool const maintext = false,
			      bool const fragile = false) const override;
	///
	bool neverIndent() const override { return true; }
	///
	bool inheritFont() const override { return false; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	bool hasFixedWidth() const override;
	///
	std::string contextMenuName() const override;
	//@}

	/// \name Public functions inherited from InsetCollapsible class
	//@{
	///
	void setButtonLabel() override;
	//@}

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override { return new InsetBox(*this); }
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
