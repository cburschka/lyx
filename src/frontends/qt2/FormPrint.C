/**
 * \file FormPrint.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include "printdlgimpl.h"
#include "FormPrint.h"
#include "Dialogs.h"
#include "gettext.h"
#include "buffer.h"
#include "lyxrc.h" 
#include "QtLyXView.h" 

#include "PrinterParams.h" 
#include "Liason.h" 
#include "BufferView.h" 
#include "support/lstrings.h"
#include "qmessagebox.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

#ifdef CXX_WORKING_NAMESPACES
using Liason::printBuffer;
using Liason::getPrinterParams;
using std::max;
#endif

FormPrint::FormPrint(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPrint.connect(slot(this, &FormPrint::show));
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
	
	int retval; 
	retval = printBuffer(lv_->buffer(), PrinterParams(dialog_->getTarget(),
		string(dialog_->getPrinter()), string(dialog_->getFile()), 
		dialog_->getWhichPages(), from, to, dialog_->getReverse(), 
		dialog_->getSort(), max(strToInt(dialog_->getCount()),1)));

	if (!retval) {
		// FIXME: should have a utility class for this
		string message(_("An error occured while printing.\n\n"));
		message += _("Check the parameters are correct.\n");
	        QMessageBox msg( _("LyX: Print Error"), message.c_str(), QMessageBox::Warning, 1,0,0);
		msg.raise();
		msg.setActiveWindow();
		msg.show();
	}
}

void FormPrint::show()
{
	if (!dialog_)
		dialog_ = new PrintDlgImpl(this, 0, _("LyX: Print"));
 
	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormPrint::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormPrint::update));
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
