// -*- C++ -*-
/**
 * \file QThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
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
	QThesaurus();
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// replace the word
	void replace();
};

#endif // QTHESAURUS_H
