// -*- C++ -*-
/**
 * \file GuiERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GuiERT_H
#define GuiERT_H

#include "GuiDialogView.h"
#include "ui_ERTUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class GuiERT;

class GuiERTDialog : public QDialog, public Ui::ERTUi {
	Q_OBJECT
public:
	GuiERTDialog(GuiERT * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiERT * form_;
};



class ControlERT;

class GuiERT : public QController<ControlERT, GuiView<GuiERTDialog> >
{
public:
	friend class GuiERTDialog;

	GuiERT(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GuiERT_H
