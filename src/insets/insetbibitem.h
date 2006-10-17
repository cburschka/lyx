// -*- C++ -*-
/**
 * \file insetbibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_BIBITEM_H
#define INSET_BIBITEM_H


#include "insetcommand.h"

/** Used to insert bibitem's information (key and label)

  Must be automatically inserted as the first object in a
  bibliography paragraph.
  */
class InsetBibitem : public InsetCommand {
public:
	///
	InsetBibitem(InsetCommandParams const &);
	///
	void read(Buffer const &, LyXLex & lex);
	///
	lyx::docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetBase::Code lyxCode() const { return InsetBase::BIBITEM_CODE; }
	///
	void setCounter(int);
	///
	int getCounter() const { return counter; }
	///
	lyx::docstring const getBibLabel() const;
	///
	int plaintext(Buffer const &, lyx::odocstream &,
		      OutputParams const &) const;
protected:
	///
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

	///
	int counter;
	///
	static int key_counter;
};


/// Return the widest label in the Bibliography.
lyx::docstring const bibitemWidest(Buffer const &);

#endif // INSET_BIBITEM_H
