// -*- C++ -*-
/**
 * \file InsetIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INDEX_H
#define INSET_INDEX_H


#include "InsetCommand.h"


namespace lyx {

class LaTeXFeatures;

/** Used to insert index labels
  */
class InsetIndex : public InsetCommand {
public:
	///
	InsetIndex(InsetCommandParams const &);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
private:
	virtual std::auto_ptr<Inset> doClone() const {
		return std::auto_ptr<Inset>(new InsetIndex(params()));
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
	Inset::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	docstring const getScreenLabel(Buffer const &) const;
private:
	virtual std::auto_ptr<Inset> doClone() const {
		return std::auto_ptr<Inset>(new InsetPrintIndex(params()));
	}
};


} // namespace lyx

#endif
