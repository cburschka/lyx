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

#ifndef INSET_CITE_H
#define INSET_CITE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "bibforms.h"
#include <vector>

class Buffer;

/** Used to insert citations  
 */
class InsetCitation: public InsetCommand {
public:
	///
	enum State {
		CANCEL,
		OK,
		DOWN,
		UP,
		DELETE,
		ADD,
		BIBBRSR,
		CITEBRSR,
		ON,
		OFF
	};
	///
	InsetCitation() : InsetCommand("cite") {}
	///
	explicit
	InsetCitation(string const & key, string const & note = string());
        ///
	~InsetCitation();
        ///
        Inset * Clone() const {
		return new InsetCitation(getContents(), getOptions());
	}
    	///
	string getScreenLabel()const;
        ///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	void callback( FD_citation_form *, State );
        ///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
	///
	struct Holder {
		InsetCitation * inset;
		BufferView * view;
	};

private:
        ///
	void setSize( FD_citation_form *, int, bool ) const;
	///
	void setBibButtons( FD_citation_form *, State ) const;
	///
	void setCiteButtons( FD_citation_form *, State ) const;
	///
	void updateBrowser( FL_OBJECT *, std::vector<string> const & ) const;
        ///	
	Holder holder;
};

#endif // INSET_CITE_H
