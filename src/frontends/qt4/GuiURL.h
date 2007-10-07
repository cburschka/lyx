// -*- C++ -*-
/**
 * \file GuiURL.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIURL_H
#define GUIURL_H

#include "GuiDialog.h"
#include "ControlCommand.h"
#include "ui_URLUi.h"
#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiURL : public GuiDialog, public Ui::URLUi, public Controller
{
	Q_OBJECT

public:
	GuiURL(LyXView & lv);

public Q_SLOTS:
	void changed_adaptor();

private:
	void closeEvent(QCloseEvent *);
	/// parent controller
	Controller & controller() { return *this; }
	///
	bool isValid();
	/// apply dialog
	void applyView();
	/// update dialog
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

#endif // GUIURL_H
