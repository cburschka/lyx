// -*- C++ -*-
/**
 * \file QAbout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMABOUT_H
#define FORMABOUT_H

#include "QDialogView.h"
#include "ui_AboutUi.h"
#include <QDialog>

namespace lyx {
namespace frontend {

class ControlAboutlyx;

class QAboutDialog : public QDialog, public Ui::QAboutUi {
	Q_OBJECT
public:
	QAboutDialog(QWidget * parent = 0)
		: QDialog(parent)
	{
		setupUi(this);
		connect(closePB, SIGNAL(clicked()), this, SLOT(reject()));
	}
};


class QAbout
	: public QController<ControlAboutlyx, QView<QAboutDialog> >
{
public:
	QAbout(Dialog &);
private:
	/// not needed
	virtual void apply() {}
	/// not needed
	virtual void update_contents() {}
	// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // FORMABOUT_H
