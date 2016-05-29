/**
 * \file GuiChanges.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiChanges.h"

#include "qt_helpers.h"

#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxtime.h"

#include "Author.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include <QDateTime>
#include <QTextBrowser>


namespace lyx {
namespace frontend {


GuiChanges::GuiChanges(GuiView & lv)
	: GuiDialog(lv, "changes", qt_("Merge Changes"))
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(nextPB, SIGNAL(clicked()), this, SLOT(nextChange()));
	connect(previousPB, SIGNAL(clicked()), this, SLOT(previousChange()));
	connect(rejectPB, SIGNAL(clicked()), this, SLOT(rejectChange()));
	connect(acceptPB, SIGNAL(clicked()), this, SLOT(acceptChange()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
	bc().addReadOnly(acceptPB);
	bc().addReadOnly(rejectPB);
}


void GuiChanges::updateContents()
{
	bool const changesPresent = buffer().areChangesPresent();
	nextPB->setEnabled(changesPresent);
	previousPB->setEnabled(changesPresent);
	changeTB->setEnabled(changesPresent);

	Change const & c = bufferview()->getCurrentChange();
	bool const changePresent = c.type != Change::UNCHANGED;
	rejectPB->setEnabled(changePresent);
	acceptPB->setEnabled(changePresent);

	QString text;
	if (changePresent) {
		QString const author =
			toqstr(buffer().params().authors().get(c.author).nameAndEmail());
		if (!author.isEmpty())
			text += qt_("Changed by %1\n\n").arg(author);

		QString const date = QDateTime::fromTime_t(c.changetime)
			                 .toString(Qt::DefaultLocaleLongDate);
		if (!date.isEmpty())
			text += qt_("Change made on %1\n").arg(date);
	}
	changeTB->setPlainText(text);
}


void GuiChanges::nextChange()
{
	dispatch(FuncRequest(LFUN_CHANGE_NEXT));
}


void GuiChanges::previousChange()
{
	dispatch(FuncRequest(LFUN_CHANGE_PREVIOUS));
}


void GuiChanges::acceptChange()
{
	dispatch(FuncRequest(LFUN_CHANGE_ACCEPT));
	nextChange();
}


void GuiChanges::rejectChange()
{
	dispatch(FuncRequest(LFUN_CHANGE_REJECT));
	nextChange();
}


Dialog * createGuiChanges(GuiView & lv) { return new GuiChanges(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiChanges.cpp"
