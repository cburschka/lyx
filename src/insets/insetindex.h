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
	~InsetIndex();
	///
	virtual InsetBase * clone() const {
		return new InsetIndex(params());
	}
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
	~InsetPrintIndex();
	///
	InsetBase * clone() const {
		return new InsetPrintIndex(params());
	}
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
