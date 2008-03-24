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
#include "MailInset.h"


namespace lyx {

class InsetBranchParams {
public:
	explicit InsetBranchParams(docstring const & b = docstring())
		: branch(b) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	docstring branch;
};


/** The Branch inset for alternative, conditional output.

*/
class InsetBranch : public InsetCollapsable {
public:
	///
	InsetBranch(Buffer const &, InsetBranchParams const &);
	///
	~InsetBranch();
	///
	docstring editMessage() const;
	///
	InsetCode lyxCode() const { return BRANCH_CODE; }
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	void setButtonLabel();
	///
	virtual ColorCode backgroundColor() const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void textString(odocstream &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetBranchParams const & params() const { return params_; }
	///
	void setParams(InsetBranchParams const & params) { params_ = params; }

	/** \returns true if params_.branch is listed as 'selected' in
	    \c buffer. This handles the case of child documents.
	 */
	bool isBranchSelected() const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	virtual void updateLabels(ParIterator const &);
	///
	bool isMacroScope() const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	virtual bool useEmptyLayout() const { return false; }
protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring name() const { return from_ascii("Branch"); }
private:
	///
	friend class InsetBranchParams;
	///
	Inset * clone() const { return new InsetBranch(*this); }
	///
	InsetBranchParams params_;
};


class InsetBranchMailer : public MailInset {
public:
	///
	InsetBranchMailer(InsetBranch & inset);
	///
	virtual Inset & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static std::string const params2string(InsetBranchParams const &);
	///
	static void string2params(std::string const &, InsetBranchParams &);

private:
	///
	static std::string const name_;
	///
	InsetBranch & inset_;
};

} // namespace lyx

#endif
