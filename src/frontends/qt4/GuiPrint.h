// -*- C++ -*-
/**
 * \file GuiPrint.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPRINT_H
#define GUIPRINT_H

#include "GuiDialog.h"
#include "ui_PrintUi.h"

#include "Dialog.h"
#include "PrinterParams.h"

namespace lyx {
namespace frontend {

class GuiPrint : public GuiDialog, public Ui::PrintUi
{
	Q_OBJECT

public:
	GuiPrint(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void browseClicked();
	void fileChanged();
	void copiesChanged(int);
	void printerChanged();
	void pagerangeChanged();

private:
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	bool canApplyToReadOnly() const { return true; }
	///
	FuncCode getLfun() const { return LFUN_BUFFER_PRINT; }

	///
	PrinterParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIPRINT_H
