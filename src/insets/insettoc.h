// -*- C++ -*-
/**
 * \file insettoc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_TOC_H
#define INSET_TOC_H


#include "insetcommand.h"

class MetricsInfo;


/** Used to insert table of contents
 */
class InsetTOC : public InsetCommand {
public:
	///
	explicit InsetTOC(InsetCommandParams const &);
	///
	~InsetTOC();
	///
	std::auto_ptr<InsetBase> clone() const;
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
	int ascii(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     LatexRunParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    LatexRunParams const &) const;
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
};

#endif
