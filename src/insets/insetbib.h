// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_BIB_H
#define INSET_BIB_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "insetcommand.h"

class Buffer;

/** Used to insert bibitem's information (key and label)

  Must be automatically inserted as the first object in a
  bibliography paragraph.
  */
class InsetBibKey : public InsetCommand {
public:
	///
	InsetBibKey(InsetCommandParams const &);
	///
	~InsetBibKey();
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	/** Currently \bibitem is used as a LyX2.x command,
	    so we need this method.
	*/
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex & lex);
	///
	virtual string const getScreenLabel(Buffer const *) const;
	///
	void edit(BufferView *, int x, int y, unsigned int button);
	///
	void edit(BufferView * bv, bool front = true);
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	/// A user can't neither insert nor delete this inset
	bool deletable() const {
		return false;
	}
	///
	void setCounter(int);
	///
	int  getCounter() const { return counter; }
	///
	string const getBibLabel() const;
	///
	struct Holder {
		InsetBibKey * inset;
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


/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(InsetCommandParams const &, bool same_id = false);
	///
	~InsetBibtex();
	///
	Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetBibtex(params(), same_id);
	}
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::BIBTEX_CODE; }
	///
	void edit(BufferView *, int x, int y, unsigned int button);
	///
	void edit(BufferView * bv, bool front = true);
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool freespace) const;
	///
	std::vector<std::pair<string,string> > const getKeys(Buffer const *) const;
	///
	std::vector<string> const getFiles(Buffer const &) const;
	///
	bool addDatabase(string const &);
	///
	bool delDatabase(string const &);
	///
	bool display() const { return true; }
	///
	struct Holder {
		InsetBibtex * inset;
		BufferView * view;
	};

private:
	///
	Holder holder;
};

#endif
