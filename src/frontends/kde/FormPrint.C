/**
 * \file FormPrint.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "printdlg.h"
#include "QtLyXView.h"
#include "PrinterParams.h"
#include "gettext.h"
#include "support/lstrings.h"
 
#include "FormPrint.h"
#include "ControlPrint.h"

using std::max;

FormPrint::FormPrint(ControlPrint & c)
	: KFormBase<ControlPrint, PrintDialog>(c)
{
}


void FormPrint::build()
{
	dialog_.reset(new PrintDialog(this, 0, _("LyX: Print")));
 
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);
}

 
void FormPrint::apply()
{
	string from;
	int to(0);

	if (strlen(dialog_->getFrom())) {
		from = dialog_->getFrom();
		if (strlen(dialog_->getTo()))
			to = strToInt(dialog_->getTo());
	}

	controller().params() = PrinterParams(dialog_->getTarget(),
		dialog_->getPrinter(), dialog_->getFile(),
		dialog_->getWhichPages(), from, to, dialog_->getReverse(),
		dialog_->getSort(), max(strToInt(dialog_->getCount()), 1));
}

 
void FormPrint::update()
{
	PrinterParams & pp(controller().params());

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
