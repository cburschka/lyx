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
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetIndex(params()));
	}
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const;
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);
	///
	~InsetPrintIndex();
	///
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetPrintIndex(params()));
	}
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	InsetOld::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
private:
	///
	mutable unsigned int center_indent_;
};

#endif
