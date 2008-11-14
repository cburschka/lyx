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
#include "lyxfind.h"
#include "LyXRC.h"

#include <QTextBrowser>


namespace lyx {
namespace frontend {

using support::bformat;

GuiChanges::GuiChanges(GuiView & lv)
	: GuiDialog(lv, "changes", qt_("Merge Changes"))
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(nextPB, SIGNAL(clicked()), this, SLOT(nextChange()));
	connect(rejectPB, SIGNAL(clicked()), this, SLOT(rejectChange()));
	connect(acceptPB, SIGNAL(clicked()), this, SLOT(acceptChange()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
	bc().addReadOnly(acceptPB);
	bc().addReadOnly(rejectPB);
}


void GuiChanges::updateContents()
{
	docstring text;
	docstring author = changeAuthor();
	docstring date = changeDate();

	if (!author.empty())
		text += bformat(_("Change by %1$s\n\n"), author);
	if (!date.empty())
		text += bformat(_("Change made at %1$s\n"), date);

	changeTB->setPlainText(toqstr(text));
}


void GuiChanges::nextChange()
{
	dispatch(FuncRequest(LFUN_CHANGE_NEXT));
}


docstring GuiChanges::changeDate() const
{
	Change const & c = bufferview()->getCurrentChange();
	if (c.type == Change::UNCHANGED)
		return docstring();

	// FIXME UNICODE
	return from_utf8(formatted_time(c.changetime, lyxrc.date_insert_format));
}


docstring GuiChanges::changeAuthor() const
{
	Change const & c = bufferview()->getCurrentChange();
	if (c.type == Change::UNCHANGED)
		return docstring();

	Author const & a = buffer().params().authors().get(c.author);

	docstring author = a.name();

	if (!a.email().empty())
		author += " (" + a.email() + ")";

	return author;
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
