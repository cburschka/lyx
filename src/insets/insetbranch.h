// -*- C++ -*-
/**
 * \file insetbranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBRANCH_H
#define INSETBRANCH_H


#include "insetcollapsable.h"

class Buffer;


class InsetBranchParams {
public:
	explicit InsetBranchParams(std::string const & b = std::string())
		: branch(b) {}
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	std::string branch;
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
	std::string const editMessage() const;
	///
	InsetBase::Code lyxCode() const { return InsetBase::BRANCH_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void setButtonLabel();
	///
	bool showInsetDialog(BufferView *) const;
	///
	int latex(Buffer const &, std::ostream &,
			OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const & runparams) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const & runparams) const;
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
	bool getStatus(LCursor &, FuncRequest const &, FuncStatus &) const;

protected:
	InsetBranch(InsetBranch const &);
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
private:
	friend class InsetBranchParams;

	virtual std::auto_ptr<InsetBase> doClone() const;

	/// used by the constructors
	void init();
	///
	InsetBranchParams params_;
};

#include "mailinset.h"

class InsetBranchMailer : public MailInset {
public:
	///
	InsetBranchMailer(InsetBranch & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
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



#endif
