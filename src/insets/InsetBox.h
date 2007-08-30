// -*- C++ -*-
/**
 * \file InsetBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBOX_H
#define INSETBOX_H

#include "InsetCollapsable.h"
#include "Length.h"
#include "MailInset.h"


namespace lyx {

class InsetBoxParams {
public:
	///
	InsetBoxParams(std::string const &);
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	std::string type;
	/// Use a parbox (true) or minipage (false)
	bool use_parbox;
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
};


/** The fbox/fancybox inset

*/
class InsetBox : public InsetCollapsable {
public:
	///
	InsetBox(BufferParams const &, std::string const &);
	///
	~InsetBox();
	///
	virtual docstring const editMessage() const;
	///
	Inset::Code lyxCode() const { return Inset::BOX_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	void setButtonLabel();
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	/// show the Box dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	DisplayType display() const { return Inline; }
	///
	bool forceDefaultParagraphs(idx_type) const;
	///
	bool neverIndent(Buffer const &) const { return true; }
	///
	bool noFontChange() const { return true; }
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetBoxParams const & params() const { return params_; }
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	enum BoxType {
		Frameless,
		Boxed,
		ovalbox,
		Ovalbox,
		Shadowbox,
		Doublebox
	};
protected:
	InsetBox(InsetBox const &);
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// Is the width forced to some value?
	virtual bool hasFixedWidth() const;
	///
	virtual docstring name() const { return from_ascii("Box"); }
private:
	friend class InsetBoxParams;

	virtual Inset * clone() const;

	/// used by the constructors
	void init();
	///
	InsetBoxParams params_;
};


class InsetBoxMailer : public MailInset {
public:
	///
	InsetBoxMailer(InsetBox & inset);
	///
	virtual Inset & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static std::string const params2string(InsetBoxParams const &);
	///
	static void string2params(std::string const &, InsetBoxParams &);

private:
	///
	static std::string const name_;
	///
	InsetBox & inset_;
};


} // namespace lyx

#endif // INSET_BOX_H
