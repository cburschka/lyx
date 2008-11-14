// -*- C++ -*-
/**
 * \file GuiVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIVSPACE_H
#define GUIVSPACE_H

#include "GuiDialog.h"
#include "ui_VSpaceUi.h"
#include "VSpace.h"

namespace lyx {
namespace frontend {

class GuiVSpace : public GuiDialog, public Ui::VSpaceUi
{
	Q_OBJECT

public:
	GuiVSpace(GuiView & lv);

private Q_SLOTS:
	///
	void change_adaptor();
	///
	void enableCustom(int);

private:
	/// Apply from dialog
	void applyView();
	/// Update the dialog
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	///
	VSpace params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVSPACE_H
