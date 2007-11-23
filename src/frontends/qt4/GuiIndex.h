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
#include "ui_IndexUi.h"

namespace lyx {
namespace frontend {

class GuiIndexDialogBase : public GuiCommand, public Ui::IndexUi
{
	Q_OBJECT

public:
	GuiIndexDialogBase(GuiView & lv, docstring const & title,
		QString const & label, std::string const & name);

private Q_SLOTS:
	void change_adaptor();
	void reject();

private:
	///
	void closeEvent(QCloseEvent * e);
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();

	///
	QString label_;
};


class GuiIndex : public GuiIndexDialogBase
{
public:
	GuiIndex(GuiView & lv);
};


class GuiLabel : public GuiIndexDialogBase
{
public:
	GuiLabel(GuiView & lv);
};


} // namespace frontend
} // namespace lyx

#endif // GUIINDEX_H
