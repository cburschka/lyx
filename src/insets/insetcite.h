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

/** Used to insert citations  
 */
class InsetCitation: public InsetCommand {
public:
	///
	explicit
	InsetCitation(string const & key, string const & note = string());
	///
	Inset * Clone() const {
		return new InsetCitation(getContents(), getOptions());
	}
	///
	string getScreenLabel() const;
	///
	Code LyxCode() const { return CITATION_CODE; }
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
private:
	///
	InsetCitation() : InsetCommand("cite") {}
};

#endif // INSET_CITE_H
