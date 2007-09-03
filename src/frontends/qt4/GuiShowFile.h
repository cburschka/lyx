// -*- C++ -*-
/**
 * \file GuiShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSHOWFILE_H
#define QSHOWFILE_H

#include "GuiDialogView.h"
#include "ControlShowFile.h"
#include "ui_ShowFileUi.h"

#include <QDialog>


namespace lyx {
namespace frontend {

class GuiShowFile;

class GuiShowFileDialog : public QDialog, public Ui::ShowFileUi
{
	Q_OBJECT
public:
	GuiShowFileDialog(GuiShowFile * form);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiShowFile * form_;
};


class GuiShowFile : public GuiView<GuiShowFileDialog>
{
public:
	friend class GuiShowFileDialog;

	GuiShowFile(GuiDialog &);
	/// parent controller
	ControlShowFile & controller()
	{ return static_cast<ControlShowFile &>(this->getController()); }
	/// parent controller
	ControlShowFile const & controller() const
	{ return static_cast<ControlShowFile const &>(this->getController()); }
private:
	/// Apply changes
	virtual void applyView() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QSHOWFILE_H
