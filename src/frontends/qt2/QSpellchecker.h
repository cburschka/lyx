// -*- C++ -*-
/**
 * \file QSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSPELLCHECKER_H
#define QSPELLCHECKER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlSpellchecker;
class QSpellcheckerDialog;


class QSpellchecker
	: public Qt2CB<ControlSpellchecker, Qt2DB<QSpellcheckerDialog> >
{
public:
	friend class QSpellcheckerDialog;

	QSpellchecker();

	/// update from controller
	void partialUpdate(int id);
private:
	void stop();
	void accept();
	void add();
	void ignore();
	void replace();
	void spellcheck();

	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QSPELLCHECKER_H
