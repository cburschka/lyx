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
	Inset * Clone(Buffer const &) const;
	/** Currently \bibitem is used as a LyX2.x command,
	    so we need this method.
	*/
        void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	virtual string const getScreenLabel() const;
        ///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
	/// A user can't neither insert nor delete this inset
	bool Deletable() const {
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
	InsetBibtex(InsetCommandParams const &);
	///
	~InsetBibtex();
	///
	Inset * Clone(Buffer const &) const {
		return new InsetBibtex(params());
	}
	///
	string const getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	Inset::Code LyxCode() const { return Inset::BIBTEX_CODE; }
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	/// 
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool freespace) const;
	///
	std::vector<std::pair<string,string> > const getKeys(Buffer const *) const;
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
