// -*- C++ -*-
/**
 * \file insetbranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
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
 	string branch;
	/// Hack -- MV
	BranchList branchlist;
 };
 
 
/** The Branch inset for alternative, conditional output.

*/
class InsetBranch : public InsetCollapsable {
public:
	///

	
 	InsetBranch(BufferParams const &, string const &);
 	/// Copy constructor
  	InsetBranch(InsetBranch const &);
  	///
	~InsetBranch();
 	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	string const editMessage() const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::BRANCH_CODE; }
	///
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void setButtonLabel();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
 	dispatch_result localDispatch(FuncRequest const &);
	///
 	int latex(Buffer const *, std::ostream &,
 			LatexRunParams const &) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool) const;
	///
	int ascii(Buffer const *, std::ostream &, int) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetBranchParams const & params() const { return params_; }
	///
	void setParams(InsetBranchParams const & params) { params_ = params; }
	
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
	InsetBranchMailer(string const & name, InsetBranch & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string(Buffer const &) const;
	///
	static string const params2string(string const &, InsetBranchParams const &);
	///
	static void string2params(string const &, InsetBranchParams &);

private:
	///
	string const name_;
	///
	InsetBranch & inset_;
};



#endif
