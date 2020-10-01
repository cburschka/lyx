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

#include "ui_NomenclUi.h"

#include "InsetParamsWidget.h"

namespace lyx {
namespace frontend {

class GuiNomenclature : public InsetParamsWidget, public Ui::NomenclUi
{
	Q_OBJECT

public:
	GuiNomenclature(QWidget * parent = 0);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const override { return NOMENCL_CODE; }
	FuncCode creationCode() const override { return LFUN_INSET_INSERT; }
	QString dialogTitle() const override { return qt_("Nomenclature Settings"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool checkWidgets(bool readonly) const override;
	bool initialiseParams(std::string const &) override;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUINOMENCLATURE_H
