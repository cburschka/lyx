/**
 * \file xforms/FormPrint.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlPrint.h"
#include "FormPrint.h"
#include "forms/form_print.h"
#include "Tooltips.h"

#include "PrinterParams.h"

#include "input_validators.h"
#include "xforms_helpers.h"

#include "support/lstrings.h"
#include FORMS_H_LOCATION

using std::make_pair;

typedef FormCB<ControlPrint, FormDB<FD_print> > base_class;

FormPrint::FormPrint()
	: base_class(_("Print")),
	  target_(2), which_pages_(2)
{}


void FormPrint::build()
{
	dialog_.reset(build_print(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	// allow controlling of input and ok/apply (de)activation
	fl_set_input_return(dialog_->input_printer, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_file, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_from_page, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_to_page, FL_RETURN_CHANGED);

	// limit these inputs to unsigned integers
	fl_set_input_filter(dialog_->input_from_page, fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_to_page, fl_unsigned_int_filter);

	// what limits (if any) make sense for these?
	fl_set_input_maxchars(dialog_->input_printer, 255);
	fl_set_input_maxchars(dialog_->input_file, 255);
	fl_set_input_maxchars(dialog_->input_from_page, 4); // 9999
	fl_set_input_maxchars(dialog_->input_to_page, 4);   // 9999

	bc().addReadOnly(dialog_->button_browse);   
	bc().addReadOnly(dialog_->check_odd_pages);   
	bc().addReadOnly(dialog_->check_even_pages);   
	bc().addReadOnly(dialog_->check_sorted_copies);
	bc().addReadOnly(dialog_->check_reverse_order);

	target_.reset();
	target_.init(dialog_->radio_printer, PrinterParams::PRINTER);
	target_.init(dialog_->radio_file,    PrinterParams::FILE);
	which_pages_.reset();
	which_pages_.init(dialog_->radio_all_pages, true);
	which_pages_.init(dialog_->radio_from_to,   false);
	
	// set up the tooltips for Destination
	string str = _("Select for printer output.");
	tooltips().init(dialog_->radio_printer, str);
	str = _("Enter printer command.");
	tooltips().init(dialog_->input_printer, str);
	str = _("Select for file output.");
	tooltips().init(dialog_->radio_file, str);
	str = _("Enter file name as print destination.");
	tooltips().init(dialog_->input_file, str);
	str = _("Browse directories for file name.");
	tooltips().init(dialog_->button_browse, str);

	// set up the tooltips for Range
	str = _("Select for printing all pages.");
	tooltips().init(dialog_->radio_all_pages, str);
	str = _("Select for printing a specific page range.");
	tooltips().init(dialog_->radio_from_to, str);
	str = _("First page.");
	tooltips().init(dialog_->input_from_page, str);
	str = _("Last page.");
	tooltips().init(dialog_->input_to_page, str);
	str = _("Print the odd numbered pages.");
	tooltips().init(dialog_->check_odd_pages, str);
	str = _("Print the even numbered pages.");
	tooltips().init(dialog_->check_even_pages, str);

	// set up the tooltips for Copies
	str = _("Number of copies to be printed.");
	tooltips().init(dialog_->counter_copies, str);
	str = _("Sort the copies.");
	tooltips().init(dialog_->check_sorted_copies, str);

	str = _("Reverse the order of the printed pages.");
	tooltips().init(dialog_->check_reverse_order, str);
}


void FormPrint::apply()
{
	PrinterParams pp;

	pp.target = static_cast<PrinterParams::Target>(target_.get());
	pp.printer_name = fl_get_input(dialog_->input_printer);
	pp.file_name = fl_get_input(dialog_->input_file);

	pp.all_pages = which_pages_.get();
	pp.from_page = 0;
	pp.to_page = 0;
	if (strlen(fl_get_input(dialog_->input_from_page)) > 0) {
		// we have at least one page requested
		pp.from_page = strToInt(fl_get_input(dialog_->input_from_page));
		if (strlen(fl_get_input(dialog_->input_to_page)) > 0) {
			// okay we have a range
			pp.to_page = strToInt(fl_get_input(dialog_->input_to_page));
		} // else we only print one page.
	}

	pp.odd_pages = static_cast<bool>(fl_get_button(dialog_->check_odd_pages));
	pp.even_pages = static_cast<bool>(fl_get_button(dialog_->check_even_pages));

	pp.count_copies = static_cast<unsigned int>(fl_get_counter_value(dialog_->counter_copies));
	pp.sorted_copies = static_cast<bool>(fl_get_button(dialog_->check_sorted_copies));

	pp.reverse_order = static_cast<bool>(fl_get_button(dialog_->check_reverse_order));

	controller().params() = pp;
}


void FormPrint::update()
{
	PrinterParams & pp = controller().params();

	target_.set(pp.target);
	fl_set_input(dialog_->input_printer, pp.printer_name.c_str());
	fl_set_input(dialog_->input_file, pp.file_name.c_str());

	// hmmm... maybe a bit weird but maybe not
	// we might just be remembering the last time this was printed.
	which_pages_.set(pp.all_pages);
	
	string const from = ( pp.from_page ? tostr(pp.from_page) : "");
	string const to   = ( pp.to_page   ? tostr(pp.to_page)   : "");
	fl_set_input(dialog_->input_from_page, from.c_str());
	fl_set_input(dialog_->input_to_page, to.c_str());

	fl_set_button(dialog_->check_odd_pages, pp.odd_pages);
	fl_set_button(dialog_->check_even_pages, pp.even_pages);
	fl_set_button(dialog_->check_reverse_order, pp.reverse_order);
	fl_set_button(dialog_->check_sorted_copies, pp.sorted_copies);

	fl_set_counter_value(dialog_->counter_copies, pp.count_copies);

	// number of copies only used when output goes to printer
	bool const enable_counter = pp.target == PrinterParams::PRINTER;
	setEnabled(dialog_->counter_copies, enable_counter);

	// sorting only used when printing more than one copy
	setEnabled(dialog_->check_sorted_copies, enable_counter && pp.count_copies > 1);
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
		// from is empty but to exists, so probably editting from
		// therefore deactivate ok and apply until form is valid again
		activate = ButtonPolicy::SMI_INVALID;
	} else {
		// both from and to are empty.  This is valid so activate
		// ok and apply but deactivate to
		fl_deactivate_object(dialog_->input_to_page);
	}

	// number of copies only used when output goes to printer
	bool const enable_counter = static_cast<bool>(fl_get_button(dialog_->radio_printer));
	setEnabled(dialog_->counter_copies, enable_counter);

	// sorting only used when printing more than one copy
	bool const enable_sorted = enable_counter
			&& static_cast<unsigned int>(fl_get_counter_value(dialog_->counter_copies)) > 1;
	setEnabled(dialog_->check_sorted_copies, enable_sorted);

	// disable OK/Apply buttons when file output is selected, but no file name entered.
	if (fl_get_button(dialog_->radio_file) && !strlen(fl_get_input(dialog_->input_file))) {
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

	// if we type input string for file or printer, select that as a target
	if (ob == dialog_->input_file && !fl_get_button(dialog_->radio_file)) {
		fl_set_button(dialog_->radio_printer, 0);
		fl_set_button(dialog_->radio_file, 1);
	} else if (ob == dialog_->input_printer && !fl_get_button(dialog_->radio_printer)) {
		fl_set_button(dialog_->radio_printer, 1);
		fl_set_button(dialog_->radio_file, 0);
	// if we type intput string for from/to, select from/to radio button
	} else if ( (ob == dialog_->input_from_page || ob == dialog_->input_to_page) &&
			!fl_get_button(dialog_->radio_from_to)) {
		fl_set_button(dialog_->radio_from_to, 1);
		fl_set_button(dialog_->radio_all_pages, 0);
	}

	return activate;
}
