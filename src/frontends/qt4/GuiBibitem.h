// -*- C++ -*-
/**
 * \file GuiBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBITEM_H
#define GUIBIBITEM_H

#include "GuiDialog.h"
#include "ui_BibitemUi.h"

#include "frontends/Dialog.h"

#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiBibitem : public GuiDialog, public Ui::BibitemUi, public Controller
{
	Q_OBJECT

public:
	GuiBibitem(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	///
	void closeEvent(QCloseEvent * e);
	/// parent controller
	Controller & controller() { return *this; }

private:
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

private:
	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBITEM_H
