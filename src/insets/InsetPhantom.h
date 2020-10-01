// -*- C++ -*-
/**
 * \file InsetPhantom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PHANTOM_H
#define INSET_PHANTOM_H

#include "InsetCollapsible.h"


namespace lyx {

class InsetPhantomParams
{
public:
	enum Type {
		Phantom,
		HPhantom,
		VPhantom
	};
	/// \c type defaults to Phantom
	InsetPhantomParams();
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Type type;
};


/////////////////////////////////////////////////////////////////////////
//
// InsetPhantom
//
/////////////////////////////////////////////////////////////////////////

/// The phantom inset
class InsetPhantom : public InsetCollapsible
{
public:
	///
	InsetPhantom(Buffer *, std::string const &);
	///
	~InsetPhantom();
	///
	static std::string params2string(InsetPhantomParams const &);
	///
	static void string2params(std::string const &, InsetPhantomParams &);
	///
	InsetPhantomParams const & params() const { return params_; }
private:
	///
	InsetCode lyxCode() const override { return PHANTOM_CODE; }
	///
	docstring layoutName() const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	void setButtonLabel() override;
	/// show the phantom dialog
	bool showInsetDialog(BufferView * bv) const override;
	///
	bool neverIndent() const override { return true; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	/// Makes no sense for XHTML.
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	Inset * clone() const override { return new InsetPhantom(*this); }
	/// used by the constructors
	void init();
	///
	std::string contextMenuName() const override;
	///
	friend class InsetPhantomParams;

	///
	InsetPhantomParams params_;
};


} // namespace lyx

#endif // INSET_PHANTOM_H
