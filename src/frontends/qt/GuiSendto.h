// -*- C++ -*-
/**
 * \file GuiSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISENDTO_H
#define GUISENDTO_H

#include "GuiDialog.h"
#include "ui_SendtoUi.h"

#include <vector>

class QListWidgetItem;

namespace lyx {

class Format;

namespace frontend {

class GuiSendTo : public GuiDialog, public Ui::SendtoUi
{
	Q_OBJECT

public:
	GuiSendTo(GuiView & lv);

private Q_SLOTS:
	void changed_adaptor();
	void slotFormatHighlighted(QListWidgetItem *) {}
	void slotFormatSelected(QListWidgetItem *) {}

private:
	///
	bool isValid() override;
	/// Apply from dialog
	void applyView() override;
	/// Update the dialog
	void updateContents() override;

	///
	bool initialiseParams(std::string const & data) override;
	///
	void paramsToDialog(Format const * format, QString const & command);
	///
	void clearParams() override {}
	///
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return true; }
	///
	FuncCode getLfun() const override { return LFUN_BUFFER_EXPORT_CUSTOM; }

private:
	///
	Format const * format_;
	///
	QString command_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISENDTO_H
