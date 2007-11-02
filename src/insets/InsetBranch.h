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

class Buffer;

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
	InsetBranch(BufferParams const &, InsetBranchParams const &);
	///
	~InsetBranch();
	///
	virtual docstring const editMessage() const;
	///
	InsetCode lyxCode() const { return BRANCH_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	void setButtonLabel();
	///
	virtual ColorCode backgroundColor() const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	void textString(Buffer const & buf, odocstream &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetBranchParams const & params() const { return params_; }
	///
	void setParams(InsetBranchParams const & params) { params_ = params; }

	/** \returns true if params_.branch is listed as 'selected' in
	    \c buffer. This handles the case of child documents.
	 */
	bool isBranchSelected(Buffer const & buffer) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	// 
	virtual void updateLabels(Buffer const &, ParIterator const &);
protected:
	///
	InsetBranch(InsetBranch const &);
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring name() const { return from_ascii("Branch"); }
private:
	friend class InsetBranchParams;

	virtual Inset * clone() const;

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
