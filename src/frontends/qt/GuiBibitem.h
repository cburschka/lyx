// -*- C++ -*-
/**
 * \file GuiBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBITEM_H
#define GUIBIBITEM_H

#include "InsetParamsWidget.h"
#include "ui_BibitemUi.h"


namespace lyx {
namespace frontend {

class GuiBibitem : public InsetParamsWidget, public Ui::BibitemUi
{
	Q_OBJECT

public:
	/// Constructor
	GuiBibitem(QWidget * parent = 0);

private:
	/// \name DialogView inherited methods
	//@{
	InsetCode insetCode() const override { return BIBITEM_CODE; }
	FuncCode creationCode() const override { return LFUN_INSET_INSERT; }
	QString dialogTitle() const override { return qt_("Bibliography Item Settings"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool checkWidgets(bool readonly) const override;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBITEM_H
