// -*- C++ -*-
/**
 * \file InsetBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_BIBITEM_H
#define INSET_BIBITEM_H


#include "InsetCommand.h"
#include "Biblio.h"


namespace lyx {

/** Used to insert bibitem's information (key and label)

  Must be automatically inserted as the first object in a
  bibliography paragraph.
  */
class InsetBibitem : public InsetCommand {
public:
	///
	InsetBibitem(InsetCommandParams const &);
	///
	void read(Buffer const &, Lexer & lex);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::BIBITEM_CODE; }
	///
	docstring const getBibLabel() const;
	///
	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;
	///
	virtual void fillWithBibKeys(Buffer const &,
		BiblioInfo &, InsetIterator const &) const;
	/// Update the counter of this inset
	virtual void updateLabels(Buffer const &, ParIterator const &);

protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	virtual std::auto_ptr<Inset> doClone() const;

	/// The label that is set by updateLabels
	docstring autolabel_;
	///
	static int key_counter;
};


/// Return the widest label in the Bibliography.
docstring const bibitemWidest(Buffer const &);


} // namespace lyx

#endif // INSET_BIBITEM_H
