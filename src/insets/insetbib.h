// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_BIB_H
#define INSET_BIB_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

class Buffer;

/** Used to insert citations  
 */
class InsetCitation: public InsetCommand {
public:
	///
	InsetCitation() : InsetCommand("cite") {}
	///
	explicit
	InsetCitation(string const & key, string const & note = string());
        ///
	~InsetCitation();
        ///
        Inset * Clone() const {
		return new InsetCitation(contents, options);
	}
    	///
	string getScreenLabel()const;
        ///
	void Edit(BufferView *, int x, int y, unsigned int button);
        ///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
	int Ascii(std::ostream &) const;
	///
	struct Holder {
		InsetCitation * inset;
		BufferView * view;
	};

private:
	///
	Holder holder;
};


/** Used to insert bibitem's information (key and label)
  
  Must be automatically inserted as the first object in a
  bibliography paragraph. 
  */
class InsetBibKey: public InsetCommand {
public:
	///
	InsetBibKey() : InsetCommand("bibitem") { counter = 1; }
	///
	explicit
	InsetBibKey(string const & key, string const & label = string());
	///
	explicit
	InsetBibKey(InsetBibKey const *);
	///
	~InsetBibKey();
	///
        Inset * Clone() const { return new InsetBibKey(this); }
	/// Currently \bibitem is used as a LyX2.x command, so we need this method.
        void Write(std::ostream &) const;
	///
	virtual string getScreenLabel() const;
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
	struct Holder {
		InsetBibKey * inset;
		BufferView * view;
	};

 private:
	///
        int counter;

	///
	Holder holder;
};


/** Used to insert BibTeX's information 
  */
class InsetBibtex: public InsetCommand {
public:
	/// 
	InsetBibtex() : InsetCommand("BibTeX") { owner = 0; }
	///
	InsetBibtex(string const & dbase, string const & style,
		    Buffer *);
	///
	~InsetBibtex();

        ///
	Inset * Clone() const {
		return new InsetBibtex(contents, options, 0);
	}
	///  
	Inset::Code LyxCode() const
	{
		return Inset::BIBTEX_CODE;
	}
	///
	string getScreenLabel() const;
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	/// 
	int Latex(std::ostream &, bool fragile, bool freespace) const;
	///
	string getKeys(char delim);
	///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
        ///
        bool addDatabase(string const &);
        ///
        bool delDatabase(string const &);
	///
	bool display() const { return true; } 

   	struct Holder {
		InsetBibtex * inset;
		BufferView * view;
	};

private:
	///
	mutable Buffer * owner;

	///
	Holder holder;
};

#endif
