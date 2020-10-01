// -*- C++ -*-
/**
 * \file GuiCompareHistory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOMPAREHISTORY_H
#define GUICOMPAREHISTORY_H

#include "GuiDialog.h"
#include "ui_CompareHistoryUi.h"


namespace lyx {
namespace frontend {


class GuiCompareHistory : public GuiDialog, public Ui::CompareHistoryUi
{
	Q_OBJECT

public:
	///
	GuiCompareHistory(GuiView & lv);

private Q_SLOTS:
	///
	void slotOK();
	///
	void slotCancel();
	///
	void slotButtonBox(QAbstractButton *);
	///
	void selectRevback();
	///
	void selectBetweenrev();
private:
	///
	void updateContents() override;
	///
	bool initialiseParams(std::string const &) override;
	///
	bool isBufferDependent() const override { return true; }
	///
	void clearParams() override {}
	///
	void dispatchParams() override {}
	///
	void enableControls();

private:
};

} // namespace frontend
} // namespace lyx

#endif // GUICOMPAREHISTORY_H
