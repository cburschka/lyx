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
#include "BranchList.h"

 struct InsetBranchParams {
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	std::string branch;
	/// Hack -- MV
	BranchList branchlist;
 };


/** The Branch inset for alternative, conditional output.

*/
class InsetBranch : public InsetCollapsable {
public:
	///
	InsetBranch(BufferParams const &, std::string const &);
	/// Copy constructor
	InsetBranch(InsetBranch const &);
	///
	~InsetBranch();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	std::string const editMessage() const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::BRANCH_CODE; }
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
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
private:
	friend class InsetBranchParams;

	/// used by the constructors
	void init();
	///
	InsetBranchParams params_;
};

#include "mailinset.h"

class InsetBranchMailer : public MailInset {
public:
	///
	InsetBranchMailer(std::string const & name, InsetBranch & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static std::string const params2string(std::string const &, InsetBranchParams const &);
	///
	static void string2params(std::string const &, InsetBranchParams &);

private:
	///
	std::string const name_;
	///
	InsetBranch & inset_;
};



#endif
