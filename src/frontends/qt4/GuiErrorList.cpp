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

#include "GuiView.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "BufferList.h"
#include "LyX.h"
#include "ParIterator.h"
#include "Text.h"
#include "TexRow.h"

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
	else if (s == "literate")
		return N_("Literate");
	else if (s == "latex")
		// This covers all LaTeX variants
		// (LaTeX, PDFLaTeX, XeTeX, LuaTeX, pLaTeX)
		return N_("LaTeX");
	return s;
}

} // namespace

namespace lyx {
namespace frontend {

GuiErrorList::GuiErrorList(GuiView & lv)
	: GuiDialog(lv, "errorlist", qt_("Error List")), buf_(0), from_master_(false)
{
	setupUi(this);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
			this, SLOT(slotButtonBox(QAbstractButton *)));
	connect(viewLogPB, SIGNAL(clicked()),
		this, SLOT(viewLog()));
	connect(showAnywayPB, SIGNAL(clicked()),
		this, SLOT(showAnyway()));
	connect(errorsLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(select()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
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


void GuiErrorList::viewLog()
{
	if (&buffer() != buf_) {
		if (!theBufferList().isLoaded(buf_))
			return;
		FuncRequest fr(LFUN_BUFFER_SWITCH, buf_->absFileName());
		dispatch(fr);
	}
	dispatch(FuncRequest(LFUN_DIALOG_SHOW, "latexlog"));
}


void GuiErrorList::showAnyway()
{
	dispatch(FuncRequest(LFUN_BUFFER_VIEW_CACHE));
}


void GuiErrorList::paramsToDialog()
{
	setTitle(toqstr(name_));
	errorsLW->clear();
	descriptionTB->setPlainText(QString());

	ErrorList const & el = errorList();
	ErrorList::const_iterator it = el.begin();
	ErrorList::const_iterator const en = el.end();
	for (; it != en; ++it)
		errorsLW->addItem(toqstr(it->error));
	errorsLW->setCurrentRow(0);
	showAnywayPB->setEnabled(
		lyx::getStatus(FuncRequest(LFUN_BUFFER_VIEW_CACHE)).enabled());
}


ErrorList const & GuiErrorList::errorList() const
{
	Buffer const * buffer = from_master_
				? bufferview()->buffer().masterBuffer()
				: &bufferview()->buffer();
	if (buffer == buf_)
		error_list_ = buffer->errorList(error_type_);

	return error_list_;
}


bool GuiErrorList::initialiseParams(string const & sdata)
{
	from_master_ = prefixIs(sdata, "from_master|");
	string error_type = sdata;
	if (from_master_)
		error_type = split(sdata, '|');
	error_type_ = error_type;
	buf_ = from_master_ ?
		bufferview()->buffer().masterBuffer()
		: &bufferview()->buffer();
	name_ = bformat(_("%1$s Errors (%2$s)"),
			        _(guiErrorType(error_type)),
				    from_utf8(buf_->absFileName()));
	paramsToDialog();
	return true;
}


bool GuiErrorList::goTo(int item)
{
	ErrorItem const & err = errorList()[item];
	if (TexRow::isNone(err.start))
		return false;

	Buffer const * errbuf = err.buffer ? err.buffer : buf_;

	if (&buffer() != errbuf) {
		if (!theBufferList().isLoaded(errbuf))
			return false;
		FuncRequest fr(LFUN_BUFFER_SWITCH, errbuf->absFileName());
		dispatch(fr);
	}
	dispatch(TexRow::goToFunc(err.start, err.end));
	return true;
}


Dialog * createGuiErrorList(GuiView & lv) { return new GuiErrorList(lv); }

} // namespace frontend
} // namespace lyx


#include "moc_GuiErrorList.cpp"
