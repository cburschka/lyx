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
#include "BufferList.h"
#include "BufferView.h"
#include "FuncRequest.h"
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

namespace {
  
string const guiErrorType(string const & s)
{
	if (s == "docbook")
		return N_("DocBook");
	if (s == "literate")
		return N_("Literate");
	if (s == "platex")
		return N_("pLaTeX");
	if (s == "latex")
		return N_("LaTeX");
	return s;
}

} // namespace anon

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
	select();
	paramsToDialog();
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


void GuiErrorList::paramsToDialog()
{
	setTitle(toqstr(name_));
	errorsLW->clear();
	descriptionTB->setPlainText(QString());

	ErrorList const & el = errorList();
	ErrorList::const_iterator it = el.begin();
	ErrorList::const_iterator end = el.end();
	for (; it != end; ++it)
		errorsLW->addItem(toqstr(it->error));
	errorsLW->setCurrentRow(0);
}


ErrorList const & GuiErrorList::errorList() const
{
	if (bufferview() && &bufferview()->buffer() == buf_)
		error_list_ = bufferview()->buffer().errorList(error_type_);
	return error_list_;
}


bool GuiErrorList::initialiseParams(string const & error_type)
{
	error_type_ = error_type;
	buf_ = &bufferview()->buffer();
	name_ = bformat(_("%1$s Errors (%2$s)"),
			_(guiErrorType(error_type)),
			from_utf8(buf_->absFileName()));
	paramsToDialog();
	return true;
}


bool GuiErrorList::goTo(int item)
{
	if (!bufferview() || &buffer() != buf_) {
		if (!theBufferList().isLoaded(buf_))
			return false;
		FuncRequest fr(LFUN_BUFFER_SWITCH, buf_->absFileName());
		dispatch(fr);
	}
	ErrorItem const & err = errorList()[item];

	if (err.par_id == -1)
		return false;

	DocIterator dit = buf_->getParFromID(err.par_id);

	if (dit == doc_iterator_end(buf_->inset())) {
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
