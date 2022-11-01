// -*- C++ -*-
/**
 * \file InsetIndexMacro.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INSETMACRO_H
#define INSET_INSETMACRO_H


#include "Inset.h"
#include "InsetCollapsible.h"


namespace lyx {

class LaTeXFeatures;

class InsetIndexMacroParams
{
public:
	enum Type {
		See,
		Seealso,
		Subentry,
		Sortkey
	};
	///
	InsetIndexMacroParams();
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Type type;
};

/////////////////////////////////////////////////////////////////////////
//
// InsetIndexMacro
//
/////////////////////////////////////////////////////////////////////////

/// Used to insert index references
class InsetIndexMacro : public InsetCollapsible
{
public:
	///
	InsetIndexMacro(Buffer *, std::string const &);
	///
	~InsetIndexMacro();
	///
	static std::string params2string(InsetIndexMacroParams const &);
	///
	static void string2params(std::string const &, InsetIndexMacroParams &);
	///
	InsetIndexMacroParams const & params() const { return params_; }
	///
	void getLatex(otexstream &, OutputParams const &) const;
	///
	int getPlaintext(odocstringstream &, OutputParams const &, size_t) const;
	///
	void getDocbook(XMLStream &, OutputParams const &) const;
	///
	bool hasNoContent() const;
private:
	///
	InsetCode lyxCode() const override;
	///
	docstring layoutName() const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	bool neverIndent() const override { return true; }
	/// We do not output anything directly to the stream
	void latex(otexstream &, OutputParams const &) const override {};
	/// We do not output anything directly to the stream
	int plaintext(odocstringstream &, OutputParams const &, size_t) const override { return 0; };
	/// We do not output anything directly to the stream
	void docbook(XMLStream &, OutputParams const &) const override {};
	/// We do not output anything directly to the stream
	docstring xhtml(XMLStream &, OutputParams const &) const override { return docstring(); };
	///
	docstring getXhtml(XMLStream &, OutputParams const &) const;
	///
	bool allowSpellCheck() const override { return false; }
	///
	bool insetAllowed(InsetCode code) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	void processLatexSorting(otexstream &, OutputParams const &,
			    docstring const, docstring const) const;
	///
	bool hasSortKey() const;
	///
	void getSortkey(otexstream &, OutputParams const &) const;
	///
	std::string contextMenuName() const override;
	///
	std::string contextMenu(BufferView const &, int, int) const override;
	///
	Inset * clone() const override { return new InsetIndexMacro(*this); }
	/// used by the constructors
	void init();
	///
	friend class InsetIndexMacroParams;

	///
	InsetIndexMacroParams params_;
};


} // namespace lyx

#endif
