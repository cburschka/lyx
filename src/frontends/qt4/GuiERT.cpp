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

#include "GuiApplication.h"
#include "GuiView.h"

#include "insets/InsetERT.h"

#include "support/gettext.h"

#include <QRadioButton>
#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiERT::GuiERT(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(collapsedRB, SIGNAL(clicked()), this, SIGNAL(changed()));
	connect(openRB, SIGNAL(clicked()), this, SIGNAL(changed()));
}


docstring GuiERT::dialogToParams() const
{
	InsetCollapsable::CollapseStatus const status = openRB->isChecked()
		? InsetCollapsable::Open : InsetCollapsable::Collapsed;
	return from_ascii(InsetERT::params2string(status));
}


void GuiERT::paramsToDialog(Inset const * inset)
{
	InsetERT const * ert = static_cast<InsetERT const *>(inset);
	// FIXME: This dialog has absolutely no value...
	BufferView const * bv = guiApp->currentView()->currentBufferView();
	InsetCollapsable::CollapseStatus status = ert->status(*bv);
	switch (status) {
		case InsetCollapsable::Open: openRB->setChecked(true); break;
		case InsetCollapsable::Collapsed: collapsedRB->setChecked(true); break;
	}
}


bool GuiERT::checkWidgets(bool readonly) const
{
	ertBG->setEnabled(!readonly);
	return InsetParamsWidget::checkWidgets();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiERT.cpp"
