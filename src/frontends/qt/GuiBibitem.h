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
	InsetCode insetCode() const { return BIBITEM_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	QString dialogTitle() const { return qt_("Bibliography Item Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBITEM_H
