// -*- C++ -*-
/**
 * \file QSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSENDTO_H
#define QSENDTO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"
#include <vector>

class ControlSendto;
class QSendtoDialog;
class Format;

/** This class provides a Qt implementation of the Custom Export Dialog.
 */
class QSendto
	: public Qt2CB<ControlSendto, Qt2DB<QSendtoDialog> >
{
public:
	///
	friend class QSendtoDialog;
	///
	QSendto();

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
