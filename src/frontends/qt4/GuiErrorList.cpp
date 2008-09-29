/**
 * \file GuiErrorList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiErrorList.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "ParIterator.h"
#include "Text.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QListWidget>
#include <QPushButton>
#include <QShowEvent>
#include <QTextBrowser>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GuiErrorList::GuiErrorList(GuiView & lv)
	: GuiDialog(lv, "errorlist", qt_("Error List"))
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
	connect(errorsLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(select()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


void GuiErrorList::showEvent(QShowEvent * e)
{
	errorsLW->setCurrentRow(0);
	select();
	e->accept();
}


void GuiErrorList::select()
{
	int const item = errorsLW->row(errorsLW->currentItem());
	if (item == -1)
		return;
	goTo(item);
	descriptionTB->setPlainText(
		toqstr(errorList()[item].description));
}


void GuiErrorList::updateContents()
{
	setTitle(toqstr(name_));
	errorsLW->clear();
	descriptionTB->setPlainText(QString());

	ErrorList::const_iterator it = errorList().begin();
	ErrorList::const_iterator end = errorList().end();
	for (; it != end; ++it)
		errorsLW->addItem(toqstr(it->error));
}


ErrorList const & GuiErrorList::errorList() const
{
	return bufferview()->buffer().errorList(error_type_);
}


bool GuiErrorList::initialiseParams(string const & error_type)
{
	error_type_ = error_type;
	Buffer const & buf = bufferview()->buffer();
	name_ = bformat(_("%1$s Errors (%2$s)"), _(error_type),
				     from_utf8(buf.absFileName()));
	return true;
}


bool GuiErrorList::goTo(int item)
{
	ErrorItem const & err = errorList()[item];

	if (err.par_id == -1)
		return false;

	Buffer const & buf = buffer();
	DocIterator dit = buf.getParFromID(err.par_id);

	if (dit == doc_iterator_end(buf.inset())) {
        // FIXME: Happens when loading a read-only doc with 
        // unknown layout. Should this be the case?
		LYXERR0("par id " << err.par_id << " not found");
		return false;
	}

	// Now make the selection.
	// if pos_end is 0, this means it is end-of-paragraph
	pos_type const s = dit.paragraph().size();
	pos_type const end = err.pos_end ? min(err.pos_end, s) : s;
	pos_type const start = min(err.pos_start, end);
	pos_type const range = end - start;
	dit.pos() = start;
	BufferView * bv = const_cast<BufferView *>(bufferview());
	// FIXME: If we used an LFUN, we would not need this line:
	bv->putSelectionAt(dit, range, false);
	bv->processUpdateFlags(Update::Force | Update::FitCursor);
	return true;
}


Dialog * createGuiErrorList(GuiView & lv) { return new GuiErrorList(lv); }

} // namespace frontend
} // namespace lyx


#include "GuiErrorList_moc.cpp"
