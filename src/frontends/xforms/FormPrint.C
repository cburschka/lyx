/* FormPrint.C
 * FormPrint Interface Class Implementation
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormPrint.h"
#include "form_print.h"
#include "input_validators.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "PrinterParams.h"
#include "Liason.h"
#include "debug.h"
#include "BufferView.h"
#include "lyx_gui_misc.h"	// WriteAlert

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

#ifdef CXX_WORKING_NAMESPACES
using Liason::printBuffer;
using Liason::getPrinterParams;
#endif


FormPrint::FormPrint(LyXView * lv, Dialogs * d)
	: FormBase(lv, d, BUFFER_DEPENDENT, _("Print"), new OkApplyCancelPolicy),
	  dialog_(0), target_(2), order_(2), which_(3)
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


void FormPrint::build()
{
	dialog_ = build_print();

	// manage the ok, apply and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();

	// allow controlling of input and ok/apply (de)activation
	fl_set_input_return(dialog_->input_printer,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_file,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_from_page,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_to_page,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_count,
			    FL_RETURN_CHANGED);

	// limit these inputs to unsigned integers
	fl_set_input_filter(dialog_->input_from_page,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_to_page,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_count,
			    fl_unsigned_int_filter);

	// what limits (if any) make sense for these?
	fl_set_input_maxchars(dialog_->input_printer, 255);
	fl_set_input_maxchars(dialog_->input_file, 255);
	fl_set_input_maxchars(dialog_->input_from_page, 4); // 9999
	fl_set_input_maxchars(dialog_->input_to_page, 4);   // 9999
	fl_set_input_maxchars(dialog_->input_count, 4);     // 9999

	target_.reset();
	target_.registerRadioButton(dialog_->radio_printer,
				    PrinterParams::PRINTER);
	target_.registerRadioButton(dialog_->radio_file,
				    PrinterParams::FILE);
	order_.reset();
	order_.registerRadioButton(dialog_->radio_order_reverse,
				   true);
	order_.registerRadioButton(dialog_->radio_order_normal,
				   false);
	which_.reset();
	which_.registerRadioButton(dialog_->radio_odd_pages,
				   PrinterParams::ODD);
	which_.registerRadioButton(dialog_->radio_even_pages,
				   PrinterParams::EVEN);
	which_.registerRadioButton(dialog_->radio_all_pages,
				   PrinterParams::ALL);
}


void FormPrint::connect()
{
	FormBase::connect();
	fl_set_form_minsize(dialog_->form,
			    dialog_->form->w,
			    dialog_->form->h);
}


FL_FORM * FormPrint::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormPrint::apply()
{
	if (!lv_->view()->available()) {
		return;
	}

	PrinterParams::WhichPages
		wp(static_cast<PrinterParams::WhichPages>(which_.getButton()));

	string from;
	int to(0);
	if (strlen(fl_get_input(dialog_->input_from_page)) > 0) {
		// we have at least one page requested
		from = fl_get_input(dialog_->input_from_page);
		if (strlen(fl_get_input(dialog_->input_to_page)) > 0) {
			// okay we have a range
			to = strToInt(fl_get_input(dialog_->input_to_page));
		} // else we only print one page.
	}

	PrinterParams::Target
		t(static_cast<PrinterParams::Target>(target_.getButton()));

	// we really should use the return value here I think.
	if (!printBuffer(lv_->buffer(),
			 PrinterParams(t,
				       string(fl_get_input(dialog_->input_printer)),
				       string(fl_get_input(dialog_->input_file)),
				       wp, from, to,
				       static_cast<bool>(order_.getButton()),
				       static_cast<bool>(fl_get_button(dialog_->
								       radio_unsorted)),
				       strToInt(fl_get_input(dialog_->input_count))))) {
		WriteAlert(_("Error:"),
			   _("Unable to print"),
			   _("Check that your parameters are correct"));
	}
}


void FormPrint::update()
{
	if (dialog_
	    && lv_->view()->available()) {
		PrinterParams pp(getPrinterParams(lv_->buffer()));

		fl_set_input(dialog_->input_printer, pp.printer_name.c_str());
		fl_set_input(dialog_->input_file, pp.file_name.c_str());

		target_.setButton(pp.target);
		order_.setButton(pp.reverse_order);
		which_.setButton(pp.which_pages);

		// hmmm... maybe a bit weird but maybe not
		// we might just be remembering the last
		// time this was printed.
		if (!pp.from_page.empty()) {
			fl_set_input(dialog_->input_from_page,
				     pp.from_page.c_str());
			// we only set the "to" page of a range
			// if there's a corresponding "from"
			fl_activate_object(dialog_->input_to_page);
			if (pp.to_page) {
				fl_set_input(dialog_->input_to_page,
					     tostr(pp.to_page).c_str());
			} else {
				fl_set_input(dialog_->input_to_page,"");
			}
		} else {
			fl_deactivate_object(dialog_->input_to_page);
			fl_set_input(dialog_->input_to_page,"");
			fl_set_input(dialog_->input_from_page,"");
		}

		fl_set_input(dialog_->input_count,
			     tostr(pp.count_copies).c_str());
	}
}


// It would be nice if we checked for cases like:
// Print only-odd-pages and from_page == an even number
//
bool FormPrint::input(long)
{
	bool activate = true;

	// using a fl_input_filter that only permits numbers no '-' or '+'
	// and the user cannot enter a negative number even if they try.
	if (strlen(fl_get_input(dialog_->input_from_page))) {
		// using a page range so activate the "to" field
		fl_activate_object(dialog_->input_to_page);
		if (strlen(fl_get_input(dialog_->input_to_page))
		    && (strToInt(fl_get_input(dialog_->input_from_page))
			> strToInt(fl_get_input(dialog_->input_to_page)))) {
			// both from and to have values but from > to
			// We could have code to silently swap these
			// values but I'll disable the ok/apply until
			// the user fixes it since they may be editting
			// one of the fields.
			activate = false;
			// set both backgrounds to red?
		}
	} else if (strlen(fl_get_input(dialog_->input_to_page))) {
		// from is empty but to exists so probably editting from
		// therefore deactivate ok and apply until form is valid again
		activate = false;
	} else {
		// both from and to are empty.  This is valid so activate
		// ok and apply but deactivate to
		fl_deactivate_object(dialog_->input_to_page);
	}

	if (fl_get_button(dialog_->radio_file)
	    && !strlen(fl_get_input(dialog_->input_file))) {
		activate = false;
	}

	// it is probably legal to have no printer name since the system will
	// have a default printer set.  Or should have.
//  	if (fl_get_button(dialog_->radio_printer)
//  	    && !strlen(fl_get_input(dialog_->input_printer))) {
//  		activate = false;
//  	}
	return activate;
}
