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
	InsetTOC(InsetCommandParams const &);
	///
	~InsetTOC();
	///
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetTOC(params()));
	}
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	dispatch_result localDispatch(FuncRequest const & cmd);
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const &, std::ostream &) const;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const;
};

#endif
