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

#ifndef GUIERT_H
#define GUIERT_H

#include "GuiDialog.h"
#include "ui_ERTUi.h"
#include "insets/InsetERT.h" // InsetERT::ERTStatus

namespace lyx {
namespace frontend {

class GuiERT : public GuiDialog, public Ui::ERTUi
{
	Q_OBJECT

public:
	GuiERT(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	void closeEvent(QCloseEvent * e);
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	///
	InsetCollapsable::CollapseStatus status() const { return status_; }
	///
	void setStatus(InsetCollapsable::CollapseStatus status) { status_ = status; }
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
private:
	///
	InsetCollapsable::CollapseStatus status_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIERT_H
