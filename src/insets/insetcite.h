// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_CITE_H
#define INSET_CITE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
class Dialogs;

/** Used to insert citations  
 */
class InsetCitation : public InsetCommand {
public:
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
	string getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
        ///
	void Edit(BufferView *, int, int, unsigned int);
private:
	///
	//InsetCitation() : InsetCommand("cite"), dialogs_(0) {}
	///
	Dialogs * dialogs_;
};

#endif // INSET_CITE_H
