// -*- C++ -*-
/**
 * \file QThesaurus.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QTHESAURUS_H
#define QTHESAURUS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlThesaurus;
class QThesaurusDialog;

///
class QThesaurus
	: public Qt2CB<ControlThesaurus, Qt2DB<QThesaurusDialog> > 
{
public:
	///
	friend class QThesaurusDialog;
	///
	QThesaurus(ControlThesaurus &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// replace the word
	void replace(); 
};

#endif // QTHESAURUS_H
