// -*- C++ -*-
/**
 * \file QWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QWRAP_H
#define QWRAP_H


#include "QDialogView.h"


class ControlWrap;
class QWrapDialog;


class QWrap : public QController<ControlWrap, QView<QWrapDialog> > {
public:
	friend class QWrapDialog;

	QWrap(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QWRAP_H
