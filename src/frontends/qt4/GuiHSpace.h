// -*- C++ -*-
/**
 * \file GuiHSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIHSPACE_H
#define GUIHSPACE_H

#include "GuiDialog.h"
#include "ui_HSpaceUi.h"
#include "insets/InsetSpace.h"

namespace lyx {
namespace frontend {

class GuiHSpace : public GuiDialog, public Ui::HSpaceUi
{
	Q_OBJECT

public:
	GuiHSpace(GuiView & lv);

private Q_SLOTS:
	///
	void change_adaptor();
	///
	void enableWidgets(int);
	///
	void patternChanged();

private:
	/// Apply from dialog
	void applyView();
	/// Update the dialog
	void updateContents();
	///
	bool isValid();
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	///
	InsetSpaceParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIHSPACE_H
