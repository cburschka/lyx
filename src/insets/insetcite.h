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
class InsetCitation : public InsetCommand {
public:
	///
	InsetCitation(InsetCommandParams const &);
	///
	Inset * Clone() const { return new InsetCitation(params()); }
	///
	string const getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
        ///
	void Edit(BufferView *, int, int, unsigned int);
};

#endif // INSET_CITE_H
