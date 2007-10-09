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
	GuiSendTo(LyXView & lv);

private Q_SLOTS:
	void changed_adaptor();
	void slotFormatHighlighted(QListWidgetItem *) {}
	void slotFormatSelected(QListWidgetItem *) {}

private:
	void closeEvent(QCloseEvent * e);
	///
	bool isValid();
	/// Apply from dialog
	void applyView();
	/// Update the dialog
	void updateContents();

	///
	std::vector<Format const *> all_formats_;
	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams() {}
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	kb_action getLfun() const { return LFUN_BUFFER_EXPORT_CUSTOM; }

	/// Return a vector of those formats that can be exported from "lyx".
	std::vector<Format const *> allFormats() const;

private:
	///
	Format const * format_;
	///
	std::string command_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISENDTO_H
