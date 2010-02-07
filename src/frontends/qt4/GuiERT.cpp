/**
 * \file GuiERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiERT.h"

#include "insets/InsetERT.h"

#include "FuncRequest.h"

#include "support/gettext.h"

#include <QRadioButton>
#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiERT::GuiERT(GuiView & lv)
	: InsetDialog(lv, ERT_CODE, LFUN_INSET_INSERT, "ert", "TeX Code Settings")
{
	setupUi(this);

	connect(collapsedRB, SIGNAL(clicked()), this, SLOT(applyView()));
	connect(openRB, SIGNAL(clicked()), this, SLOT(applyView()));
}


void GuiERT::enableView(bool enable)
{
	collapsedRB->setEnabled(enable);
	openRB->setEnabled(enable);
}


docstring GuiERT::dialogToParams() const
{
	InsetCollapsable::CollapseStatus status = openRB->isChecked()
		? InsetCollapsable::Open : InsetCollapsable::Collapsed;
	return from_ascii(InsetERT::params2string(status));
}


void GuiERT::paramsToDialog(Inset const * inset)
{
	InsetERT const * ert = static_cast<InsetERT const *>(inset);
	InsetCollapsable::CollapseStatus status = ert->status(*bufferview());
	switch (status) {
		case InsetCollapsable::Open: openRB->setChecked(true); break;
		case InsetCollapsable::Collapsed: collapsedRB->setChecked(true); break;
	}
}


Dialog * createGuiERT(GuiView & lv) { return new GuiERT(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiERT.cpp"
