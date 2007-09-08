// -*- C++ -*-
/**
 * \file GuiIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIINDEX_H
#define GUIINDEX_H

#include "GuiDialog.h"
#include "ControlCommand.h"
#include "ui_IndexUi.h"

namespace lyx {
namespace frontend {

class GuiIndexDialogBase : public GuiDialog, public Ui::IndexUi
{
	Q_OBJECT

public:
	GuiIndexDialogBase(LyXView & lv, docstring const & title,
		QString const & label, std::string const & name);

private Q_SLOTS:
	void change_adaptor();
	void reject();

private:
	///
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlCommand & controller() const;
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void update_contents();

	///
	QString label_;
};


class GuiIndexDialog : public GuiIndexDialogBase
{
public:
	GuiIndexDialog(LyXView & lv);
};


class GuiLabelDialog : public GuiIndexDialogBase
{
public:
	GuiLabelDialog(LyXView & lv);
};


} // namespace frontend
} // namespace lyx

#endif // GUIINDEX_H
