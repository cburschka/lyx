// -*- C++ -*-
/**
 * \file insetbibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_BIBITEM_H
#define INSET_BIBITEM_H


#include "insetcommand.h"

class Buffer;

/** Used to insert bibitem's information (key and label)

  Must be automatically inserted as the first object in a
  bibliography paragraph.
  */
class InsetBibitem : public InsetCommand {
public:
	///
	InsetBibitem(InsetCommandParams const &);
	///
	~InsetBibitem();
	///
	Inset * clone(Buffer const &) const;
	///
	//Inset * clone(Buffer const &, bool same_id) const;
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	/** Currently \bibitem is used as a LyX2.x command,
	    so we need this method.
	*/
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex & lex);
	///
	virtual string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::BIBITEM_CODE; }
	/// keep .lyx format compatible
	bool directWrite() const { return true; }
	///
	void setCounter(int);
	///
	int getCounter() const { return counter; }
	///
	string const getBibLabel() const;
	///
	struct Holder {
		InsetBibitem * inset;
		BufferView * view;
	};

private:
	///
	int counter;
	///
	Holder holder;
	///
	static int key_counter;
};

#endif // INSET_BIBITEM_H
