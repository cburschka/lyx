// -*- C++ -*-
/**
 * \file QSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSENDTO_H
#define QSENDTO_H

#include "QDialogView.h"

#include <vector>

class ControlSendto;
class QSendtoDialog;
class Format;

/** This class provides a Qt implementation of the Custom Export Dialog.
 */
class QSendto
	: public QController<ControlSendto, QView<QSendtoDialog> >
{
public:
	///
	friend class QSendtoDialog;
	///
	QSendto(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update_contents();
	/// Build the dialog
	virtual void build_dialog();
	///
	std::vector<Format const *> all_formats_;
};

#endif // QSENDTO_H
