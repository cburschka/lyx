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


#include "QDialogView.h"


class ControlThesaurus;
class QThesaurusDialog;

///
class QThesaurus
	: public QController<ControlThesaurus, QView<QThesaurusDialog> >
{
public:
	///
	friend class QThesaurusDialog;
	///
	QThesaurus(Dialog &);
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
