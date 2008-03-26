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
#include "FuncRequest.h"

#include "support/gettext.h"

#include <QRadioButton>
#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiERT::GuiERT(GuiView & lv)
	: GuiDialog(lv, "ert", qt_("TeX Code Settings")), status_(InsetERT::Collapsed)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(collapsedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(openRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
}


void GuiERT::change_adaptor()
{
	changed();
}


void GuiERT::applyView()
{
	if (openRB->isChecked())
		status_ = Inset::Open;
	else
		status_ = Inset::Collapsed;
}


void GuiERT::updateContents()
{
	switch (status_) {
		case InsetERT::Open: openRB->setChecked(true); break;
		case InsetERT::Collapsed: collapsedRB->setChecked(true); break;
	}
}


bool GuiERT::initialiseParams(string const & data)
{
	InsetERT::string2params(data, status_);
	return true;
}


void GuiERT::clearParams()
{
	status_ = InsetERT::Collapsed;
}


void GuiERT::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetERT::params2string(status_)));
}


Dialog * createGuiERT(GuiView & lv) { return new GuiERT(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiERT_moc.cpp"
