// -*- C++ -*-
/**
 * \file InsetBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBRANCH_H
#define INSETBRANCH_H

#include "InsetCollapsible.h"

namespace lyx {

class InsetBranchParams {
public:
	///
	explicit InsetBranchParams(docstring const & b = docstring())
		: branch(b), inverted(false) {}
	InsetBranchParams(docstring const & b, bool i)
		: branch(b), inverted(i) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	docstring branch;
	///
	bool inverted;
};


/////////////////////////////////////////////////////////////////////////
//
// InsetBranch
//
/////////////////////////////////////////////////////////////////////////

/// The Branch inset for alternative, conditional output.

class InsetBranch : public InsetCollapsible
{
public:
	///
	InsetBranch(Buffer *, InsetBranchParams const &);

	///
	static std::string params2string(InsetBranchParams const &);
	///
	static void string2params(std::string const &, InsetBranchParams &);
	///
	docstring branch() const { return params_.branch; }
	///
	void rename(docstring const & newname) { params_.branch = newname; }
	///
	InsetBranchParams const & params() const { return params_; }

private:
	///
	InsetCode lyxCode() const override { return BRANCH_CODE; }
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	docstring const buttonLabel(BufferView const &) const override;
	///
	ColorCode backgroundColor(PainterInfo const &) const override;
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
	void toString(odocstream &) const override;
	///
	void forOutliner(docstring &, size_t const, bool const) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	std::string contextMenuName() const override;
	///
	void updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted = false) override;

	/** \returns true if params_.branch is listed as 'selected' in
		\c buffer. \p child only checks within child documents.
	 */
	bool isBranchSelected(bool const child = false) const;
	/*!
	 * Is the content of this inset part of the output document?
	 *
	 * Note that Branch insets are considered part of the
	 * document when they are selected XOR inverted.
	 */
	bool producesOutput() const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	bool isMacroScope() const override;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	bool usePlainLayout() const override { return false; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	docstring layoutName() const override { return from_ascii("Branch:") + branch(); }
	///
	Inset * clone() const override { return new InsetBranch(*this); }

	///
	friend class InsetBranchParams;
	///
	InsetBranchParams params_;
};

} // namespace lyx

#endif
