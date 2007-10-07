// -*- C++ -*-
/**
 * \file GuiNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUINOMENCL_H
#define GUINOMENCL_H

#include "GuiDialog.h"
#include "ui_NomenclUi.h"

namespace lyx {
namespace frontend {

class GuiNomenclature : public GuiCommand, public Ui::NomenclUi
{
	Q_OBJECT

public:
	GuiNomenclature(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void reject();

private:
	///
	void closeEvent(QCloseEvent * e);
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // GUINOMENCL_H
