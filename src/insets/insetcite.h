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
#include <sigc++/signal_system.h>

class Dialogs;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
#endif


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
	///
	Signal0<void> hide;
};

#endif // INSET_CITE_H
