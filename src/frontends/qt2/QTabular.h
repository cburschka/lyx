// -*- C++ -*-
/**
 * \file QTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Juergen Spitzmueller
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTABULAR_H
#define QTABULAR_H


#include "QDialogView.h"


class ControlTabular;
class QTabularDialog;


class QTabular :
	public QController<ControlTabular, QView<QTabularDialog> >
{
public:
	friend class QTabularDialog;

	QTabular(Dialog &);

protected:
	virtual bool isValid();

private:
	/// We can't use this ...
	virtual void apply() {}
	/// update borders
	virtual void update_borders();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	/// save some values before closing the gui
	virtual void closeGUI();
};

#endif // QTABULAR_H
