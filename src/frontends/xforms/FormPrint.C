/*
 * \file xforms/FormPrint.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlPrint.h"
#include "FormPrint.h"
#include "form_print.h"
#include "input_validators.h"
#include "support/lstrings.h"

#include "lyxrc.h" // needed by PrinterParams
#include "PrinterParams.h"

#include "LyXView.h"
#include "xforms_helpers.h"     // for browseFile

/*
#include "LyXView.h"
#include "Dialogs.h"
#include "Liason.h"
#include "debug.h"
#include "BufferView.h"
*/

//using Liason::printBuffer;
//using Liason::getPrinterParams;
using std::make_pair;

typedef FormCB<ControlPrint, FormDB<FD_form_print> > base_class;

FormPrint::FormPrint(ControlPrint & c)
	: base_class(c, _("Print")),
	  target_(2), order_(2), which_(3)
{}


void FormPrint::build()
{
	dialog_.reset(build_print());

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

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

	setPrehandler(dialog_->input_printer);
	setPrehandler(dialog_->input_file);
	setPrehandler(dialog_->input_from_page);
	setPrehandler(dialog_->input_to_page);
	setPrehandler(dialog_->input_count);

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


void FormPrint::apply()
{
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

	PrinterParams const pp(t,
			       string(fl_get_input(dialog_->input_printer)),
			       string(fl_get_input(dialog_->input_file)),
			       wp, from, to,
			       static_cast<bool>(order_.getButton()),
			       !static_cast<bool>(fl_get_button(dialog_->check_collated)),
			       strToInt(fl_get_input(dialog_->input_count)));

	controller().params() = pp;
}


void FormPrint::update()
{
	PrinterParams & pp = controller().params();

	fl_set_input(dialog_->input_printer, pp.printer_name.c_str());
	fl_set_input(dialog_->input_file, pp.file_name.c_str());

	target_.setButton(pp.target);
	order_.setButton(pp.reverse_order);
	which_.setButton(pp.which_pages);

	// hmmm... maybe a bit weird but maybe not
	// we might just be remembering the last
	// time this was printed.
	if (!pp.from_page.empty()) {
		fl_set_input(dialog_->input_from_page, pp.from_page.c_str());

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

	fl_set_input(dialog_->input_count, tostr(pp.count_copies).c_str());
}


// It would be nice if we checked for cases like:
// Print only-odd-pages and from_page == an even number
//
ButtonPolicy::SMInput FormPrint::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_VALID;

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
			activate = ButtonPolicy::SMI_INVALID;
			// set both backgrounds to red?
		}
	} else if (strlen(fl_get_input(dialog_->input_to_page))) {
		// from is empty but to exists so probably editting from
		// therefore deactivate ok and apply until form is valid again
		activate = ButtonPolicy::SMI_INVALID;
	} else {
		// both from and to are empty.  This is valid so activate
		// ok and apply but deactivate to
		fl_deactivate_object(dialog_->input_to_page);
	}

	if (fl_get_button(dialog_->radio_file)
	    && !strlen(fl_get_input(dialog_->input_file))) {
		activate = ButtonPolicy::SMI_INVALID;
	}

	if (ob == dialog_->button_browse) {
		// Get the filename from the dialog
		string const in_name = fl_get_input(dialog_->input_file);
		string const out_name = controller().Browse(in_name);

		// Save the filename to the dialog
		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(dialog_->input_file, out_name.c_str());
			input(0, 0);
		}

		// select the file radio
		if (!out_name.empty()) {
			fl_set_button(dialog_->radio_file, 1);
			fl_set_button(dialog_->radio_printer, 0);
		}
	}

	// if we type into file, select that as a target
	if (ob == dialog_->input_file && fl_get_button(dialog_->radio_printer)
	    && strlen(fl_get_input(dialog_->input_file))) {
		fl_set_button(dialog_->radio_file, 1);
		fl_set_button(dialog_->radio_printer, 0);
	} else if (ob == dialog_->input_printer) {
		fl_set_button(dialog_->radio_file, 0);
		fl_set_button(dialog_->radio_printer, 1);
	}

	return activate;
}
