// -*- C++ -*-
/**
 * \file insetindex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INDEX_H
#define INSET_INDEX_H


#include "insetcommand.h"

class LaTeXFeatures;

/** Used to insert index labels
  */
class InsetIndex : public InsetCommand {
public:
	///
	InsetIndex(InsetCommandParams const &);
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetBase::Code lyxCode() const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const {
		return std::auto_ptr<InsetBase>(new InsetIndex(params()));
	}
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	InsetBase::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	std::string const getScreenLabel(Buffer const &) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const {
		return std::auto_ptr<InsetBase>(new InsetPrintIndex(params()));
	}
};

#endif
