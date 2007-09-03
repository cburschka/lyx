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

#ifndef GUIABOUT_H
#define GUIABOUT_H

#include "GuiDialogView.h"
#include "ControlAboutlyx.h"
#include "ui_AboutUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

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


class GuiAbout : public GuiView<GuiAboutDialog>
{
public:
	GuiAbout(GuiDialog &);
	/// parent controller
	ControlAboutlyx & controller()
	{ return static_cast<ControlAboutlyx &>(this->getController()); }
	/// parent controller
	ControlAboutlyx const & controller() const
	{ return static_cast<ControlAboutlyx const &>(this->getController()); }
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

#endif // GUIABOUT_H
