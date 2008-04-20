// -*- C++ -*-
/**
 * \file GuiLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILABEL_H
#define GUILABEL_H

#include "GuiCommand.h"
#include "ui_LabelUi.h"

namespace lyx {
namespace frontend {

class GuiLabel : public GuiCommand, public Ui::LabelUi
{
	Q_OBJECT

public:
	GuiLabel(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void reject();

private:
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // GUILABEL_H
