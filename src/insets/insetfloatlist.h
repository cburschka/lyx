// -*- C++ -*-
/**
 * \file insetfloatlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FLOATLIST_H
#define INSET_FLOATLIST_H


#include "insetcommand.h"

/** Used to insert table of contents
 */
class InsetFloatList : public InsetCommand {
public:
	///
	InsetFloatList();
	///
	InsetFloatList(std::string const & type);
	///
	~InsetFloatList();
	///
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetFloatList(getCmdName()));
	}
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex &);
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     LatexRunParams const &) const { return 0; }
	///
	int docbook(Buffer const &, std::ostream &,
		    LatexRunParams const &) const { return 0; }
	///
	int ascii(Buffer const &, std::ostream &,
		  LatexRunParams const & runparams) const;
	///
	void validate(LaTeXFeatures & features) const;
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
};

#endif
