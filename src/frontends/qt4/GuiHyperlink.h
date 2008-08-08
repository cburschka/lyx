// -*- C++ -*-
/**
 * \file GuiHyperlink.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIHYPERLINK_H
#define GUIHYPERLINK_H

#include "GuiDialog.h"
#include "ui_HyperlinkUi.h"

#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiHyperlink : public GuiDialog, public Ui::HyperlinkUi
{
	Q_OBJECT

public:
	/// Constructor
	GuiHyperlink(GuiView & lv);

public Q_SLOTS:
	void changed_adaptor();

private:
	///
	bool isValid();
	/// apply dialog
	void applyView();
	/// update dialog
	void updateContents() { };
	///
	bool initialiseParams(std::string const & data);
	///
	void paramsToDialog(InsetCommandParams const & icp);
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

#endif // GUIHYPERLINK_H
