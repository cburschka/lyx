// -*- C++ -*-
/**
 * \file GuiWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIWRAP_H
#define GUIWRAP_H

#include "GuiDialog.h"
#include "ui_WrapUi.h"

#include "insets/InsetWrap.h"

namespace lyx {
namespace frontend {

class GuiWrap : public GuiDialog, public Ui::WrapUi
{
	Q_OBJECT

public:
	GuiWrap(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	/// Dialog inherited methods
	//@{
	void applyView();
	void updateContents() {}
	bool initialiseParams(std::string const & data);
	void clearParams();
	void dispatchParams();
	bool isBufferDependent() const { return true; }
	//@}

	///
	void paramsToDialog(InsetWrapParams const & params);

	///
	InsetWrapParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIWRAP_H
