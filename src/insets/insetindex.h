// -*- C++ -*-
/**
 * \file insetindex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_INDEX_H
#define INSET_INDEX_H


#include "insetcommand.h"

struct LaTeXFeatures;

/** Used to insert index labels
  */
class InsetIndex : public InsetCommand {
public:
	///
	InsetIndex(InsetCommandParams const &);
	///
	//InsetIndex(InsetCommandParams const &, bool same_id);
	///
	~InsetIndex();
	///
	virtual Inset * clone(Buffer const &) const {
		return new InsetIndex(params());
	}
	///
	//virtual Inset * clone(Buffer const &, bool same_id) const {
	//	return new InsetIndex(params(), same_id);
	//}
	///
	dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);
	///
	//InsetPrintIndex(InsetCommandParams const &, bool same_id);
	///
	~InsetPrintIndex();
	///
	Inset * clone(Buffer const &) const {
		return new InsetPrintIndex(params());
	}
	///
	//Inset * clone(Buffer const &, bool same_id) const {
	//	return new InsetPrintIndex(params(), same_id);
	//}
	///
	//dispatch_result localDispatch(FuncRequest const & cmd);
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	bool display() const { return true; }
	///
	Inset::Code lyxCode() const;
	///
	string const getScreenLabel(Buffer const *) const;
	///
	virtual bool needFullRow() const { return true; }
};

#endif
