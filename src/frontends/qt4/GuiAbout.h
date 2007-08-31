// -*- C++ -*-
/**
 * \file GuiAbout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMABOUT_H
#define FORMABOUT_H

#include "GuiDialogView.h"
#include "ui_AboutUi.h"
#include <QDialog>

namespace lyx {
namespace frontend {

class ControlAboutlyx;

class GuiAboutDialog : public QDialog, public Ui::AboutUi {
	Q_OBJECT
public:
	GuiAboutDialog(QWidget * parent = 0)
		: QDialog(parent)
	{
		setupUi(this);
		connect(closePB, SIGNAL(clicked()), this, SLOT(reject()));
	}
};


class GuiAbout
	: public QController<ControlAboutlyx, GuiView<GuiAboutDialog> >
{
public:
	GuiAbout(Dialog &);
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
