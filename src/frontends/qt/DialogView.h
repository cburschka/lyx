// -*- C++ -*-
/**
 * \file DialogView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOGVIEW_H
#define DIALOGVIEW_H

#include "Dialog.h"
#include "GuiView.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class DialogView : public QDialog, public Dialog
{
	Q_OBJECT

public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	/// \param title is the window title used for decoration.
	DialogView(GuiView & lv, QString const & name, QString const & title);

	QWidget * asQWidget() override { return this; }
	QWidget const * asQWidget() const override { return this; }

protected:
	/// \name Dialog inherited methods
	//@{
	void applyView() override {}
	bool initialiseParams(std::string const & /*data*/) override { return true; }
	void clearParams() override {}
	bool needBufferOpen() const override { return isBufferDependent(); }
	//@}
	/// Any dialog that overrides this method should make sure to call it.
	void closeEvent(QCloseEvent * ev);
	/// Any dialog that overrides this method should make sure to call it.
	void hideEvent(QHideEvent * ev);

protected Q_SLOTS:
	void onBufferViewChanged() override {};
};

} // namespace frontend
} // namespace lyx

#endif // DIALOGVIEW_H
