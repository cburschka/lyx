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

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

struct LaTeXFeatures;

/** Used to insert index labels
  */
class InsetIndex : public InsetCommand {
public:
	///
	InsetIndex(InsetCommandParams const &, bool same_id = false);
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetIndex(params(), same_id);
	}
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	void edit(BufferView *, int, int, mouse_button::state);
	///
	void edit(BufferView * bv, bool front = true);
	///
	Inset::Code lyxCode() const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &, bool same_id = false);
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetPrintIndex(params(), same_id);
	}
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	void edit(BufferView *, int, int, mouse_button::state) {}
	///
	void edit(BufferView *, bool = true) {}
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
