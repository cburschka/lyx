/**
 * \file FormPrint.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "printdlg.h"
#include "Dialogs.h"
#include "FormPrint.h"
#include "gettext.h"
#include "buffer.h"
#include "lyxrc.h" 
#include "QtLyXView.h"
#include "PrinterParams.h" 
#include "Liason.h" 
#include "BufferView.h" 
#include "support/lstrings.h"

#include <kmsgbox.h>

using Liason::printBuffer;
using Liason::getPrinterParams;
using std::max;

FormPrint::FormPrint(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
	d->showPrint.connect(SigC::slot(this, &FormPrint::show));
}


FormPrint::~FormPrint()
{
	delete dialog_;
}


// we can safely ignore the parameter because we can always update
void FormPrint::update(bool)
{
	if (!lv_->view()->available())
	       return;

	PrinterParams pp(getPrinterParams(lv_->buffer()));

	dialog_->setTarget(pp.target);
       	dialog_->setPrinter(pp.printer_name.c_str());
       	dialog_->setFile(pp.file_name.c_str());
	dialog_->setWhichPages(pp.which_pages);
        dialog_->setReverse(pp.reverse_order);
	dialog_->setSort(pp.unsorted_copies);
	dialog_->setCount(pp.count_copies);
	
       	if (!pp.from_page.empty()) {
		dialog_->setFrom(pp.from_page.c_str());
	       	if (pp.to_page)
			dialog_->setTo(tostr(pp.to_page).c_str());
		else
			dialog_->setTo("");
	} else {
		dialog_->setFrom("");
		dialog_->setTo("");
	}
}

 
void FormPrint::print()
{
	if (!lv_->view()->available())
	       return;

	string from;
	int to(0);

	if (strlen(dialog_->getFrom())) {
		from = dialog_->getFrom();
		if (strlen(dialog_->getTo()))
			to = strToInt(dialog_->getTo());
	}
	
	int retval = printBuffer(lv_->buffer(), PrinterParams(dialog_->getTarget(),
		string(dialog_->getPrinter()), string(dialog_->getFile()), 
		dialog_->getWhichPages(), from, to, dialog_->getReverse(), 
		dialog_->getSort(), max(strToInt(dialog_->getCount()),1)));

	if (!retval) {
		// FIXME: should have a utility class for this
		string message(_("An error occured while printing.\n\n"));
		message += _("Check the parameters are correct.\n");
		KMsgBox msg(0, _("LyX: Print Error"), message.c_str(), KMsgBox::EXCLAMATION, _("&OK"));
		msg.raise();
		msg.setActiveWindow();
		msg.show();
	}
}


void FormPrint::show()
{
	if (!dialog_)
		dialog_ = new PrintDialog(this, 0, _("LyX: Print"));
 
	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(SigC::slot(this, &FormPrint::hide));
		u_ = d_->updateBufferDependent.connect(SigC::slot(this, &FormPrint::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();
 
	update();
	dialog_->show();
}


void FormPrint::close()
{
	h_.disconnect();
	u_.disconnect();
}

 
void FormPrint::hide()
{
	dialog_->hide();
	close();
}
