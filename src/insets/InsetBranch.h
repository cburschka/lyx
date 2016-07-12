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

#include "InsetCollapsable.h"

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

class InsetBranch : public InsetCollapsable
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
	InsetCode lyxCode() const { return BRANCH_CODE; }
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	docstring const buttonLabel(BufferView const &) const;
	///
	ColorCode backgroundColor(PainterInfo const &) const;
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
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t const, bool const) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	std::string contextMenuName() const;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
	///
	void setParams(InsetBranchParams const & params) { params_ = params; }

	/** \returns true if params_.branch is listed as 'selected' in
		\c buffer. \p child only checks within child documents.
	 */
	bool isBranchSelected(bool const child = false) const;
	///
	bool isBranchActive(bool const child = false) const
		// XOR
		{ return isBranchSelected(child) != params_.inverted; }
	/*!
	 * Is the content of this inset part of the output document?
	 *
	 * Note that Branch insets are only considered part of the
	 * document when they are selected.
	 */
	bool producesOutput() const { return isBranchSelected(); }
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	bool isMacroScope() const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	bool usePlainLayout() const { return false; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring layoutName() const { return from_ascii("Branch:") + branch(); }
	///
	Inset * clone() const { return new InsetBranch(*this); }

	///
	friend class InsetBranchParams;
	///
	InsetBranchParams params_;
};

} // namespace lyx

#endif
