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
	InsetCode lyxCode() const { return PHANTOM_CODE; }
	///
	docstring layoutName() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	void setButtonLabel();
	/// show the phantom dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	bool neverIndent() const { return true; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	void docbook(XMLStream &, OutputParams const &) const;
	/// Makes no sense for XHTML.
	docstring xhtml(XMLStream &, OutputParams const &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetPhantom(*this); }
	/// used by the constructors
	void init();
	///
	std::string contextMenuName() const;
	///
	friend class InsetPhantomParams;

	///
	InsetPhantomParams params_;
};


} // namespace lyx

#endif // INSET_PHANTOM_H
