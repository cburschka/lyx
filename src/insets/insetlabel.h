// -*- C++ -*-
/**
 * \file insetlabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LABEL_H
#define INSET_LABEL_H


#include "insetcommand.h"

class InsetLabel : public InsetCommand {
public:
	///
	InsetLabel(InsetCommandParams const &);
	///
	~InsetLabel();
	///
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetLabel(params()));
	}
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const &) const { return getContents(); }
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::LABEL_CODE; }
	///
	void getLabelList(std::vector<string> &) const;
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const &, std::ostream &) const;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const;
};

#endif
