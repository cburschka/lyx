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

#include "ui_ShowFileUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class GuiShowFile;

class GuiShowFileDialog : public QDialog, public Ui::ShowFileUi {
	Q_OBJECT
public:
	GuiShowFileDialog(GuiShowFile * form);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiShowFile * form_;
};


class ControlShowFile;

class GuiShowFile
	: public QController<ControlShowFile, GuiView<GuiShowFileDialog> >
{
public:
	friend class GuiShowFileDialog;

	GuiShowFile(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QSHOWFILE_H
