// -*- C++ -*-
/**
 * \file QSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSPELLCHECKER_H
#define QSPELLCHECKER_H

#include "QDialogView.h"

class ControlSpellchecker;
class QSpellcheckerDialog;


class QSpellchecker
	: public QController<ControlSpellchecker, QView<QSpellcheckerDialog> >
{
public:
	friend class QSpellcheckerDialog;

	QSpellchecker(Dialog &);

	/// update from controller
	void partialUpdate(int id);
private:
	void accept();
	void add();
	void ignore();
	void replace();

	/// Apply changes
	virtual void apply() {}
	///
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QSPELLCHECKER_H
