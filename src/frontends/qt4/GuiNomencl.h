// -*- C++ -*-
/**
 * \file GuiNomenclature.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUINOMENCLATURE_H
#define GUINOMENCLATURE_H

#include "GuiDialog.h"
#include "ui_NomenclUi.h"

#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiNomenclature : public GuiDialog, public Ui::NomenclUi
{
	Q_OBJECT

public:
	GuiNomenclature(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void reject();

private:
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents() {}
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

#endif // GUINOMENCLATURE_H
