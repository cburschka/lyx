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


typedef FormCB<ControlPrint, FormDB<FD_print> > base_class;

FormPrint::FormPrint()
	: base_class(_("Print"))
{}


void FormPrint::build()
{
	dialog_.reset(build_print(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_printer);
	setPrehandler(dialog_->input_file);
	setPrehandler(dialog_->input_from_page);
	setPrehandler(dialog_->input_to_page);

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

	target_.init(dialog_->radio_printer, PrinterParams::PRINTER);
	target_.init(dialog_->radio_file,    PrinterParams::FILE);

	all_pages_.init(dialog_->radio_all_pages, true);
	all_pages_.init(dialog_->radio_from_to, false);

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
	pp.printer_name = getString(dialog_->input_printer);
	pp.file_name = getString(dialog_->input_file);

	pp.all_pages = static_cast<bool>(all_pages_.get());

	pp.from_page = pp.to_page = 0;
	if (!getString(dialog_->input_from_page).empty()) {
		// we have at least one page requested
		pp.from_page = strToInt(fl_get_input(dialog_->input_from_page));
		if (!getString(dialog_->input_to_page).empty()) {
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
	all_pages_.set(pp.all_pages);
	
	string const from = ( pp.from_page ? tostr(pp.from_page) : string() );
	string const to   = ( pp.to_page   ? tostr(pp.to_page)   : string() );
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

	// reset input fields to valid input
	input(0, 0);
}


ButtonPolicy::SMInput FormPrint::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_browse) {
		// Get the filename from the dialog
		string const in_name = getString(dialog_->input_file);
		string const out_name = controller().Browse(in_name);

		// Save the filename to the dialog
		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(dialog_->input_file, out_name.c_str());
		}

		// select the file radio
		if (!out_name.empty()) {
			target_.set(dialog_->radio_file);
		}

	// if we type input string for file or printer, select that as a target
	} else if (ob == dialog_->input_file && !fl_get_button(dialog_->radio_file)) {
		target_.set(dialog_->radio_file);

	} else if (ob == dialog_->input_printer && !fl_get_button(dialog_->radio_printer)) {
		target_.set(dialog_->radio_printer);

	// if we type into 'from/to' fields, then select 'from/to' radio button
	} else if ((ob == dialog_->input_from_page || ob == dialog_->input_to_page) &&
			!fl_get_button(dialog_->radio_from_to)) {
		all_pages_.set(dialog_->radio_from_to);
	}

	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_VALID;

	// disable OK/Apply buttons when file output is selected, but no file name entered
	if (fl_get_button(dialog_->radio_file) && getString(dialog_->input_file).empty()) {
			activate = ButtonPolicy::SMI_INVALID;
	}

	// check 'from' and 'to' fields only when 'from/to' radio button is selected
	if (fl_get_button(dialog_->radio_from_to)) {
		char const * from = fl_get_input(dialog_->input_from_page);
		char const * to = fl_get_input(dialog_->input_to_page);
		bool const from_input = static_cast<bool>(*from);
		bool const to_input = static_cast<bool>(*to);

		setEnabled(dialog_->input_to_page, from_input);
		if (!from_input || (to_input && strToInt(from) > strToInt(to))) {
			// Invalid input. Either 'from' is empty, or 'from' > 'to'.
			// Probably editting these fields, so deactivate OK/Apply until input is valid again.
			activate = ButtonPolicy::SMI_INVALID;
		} else if (!to_input || strToInt(from) == strToInt(to)) {
			// Valid input. Either there's only 'from' input, or 'from' == 'to'.
			// Deactivate OK/Apply if odd/even selection implies no pages.
			bool const odd_pages = static_cast<bool>(fl_get_button(dialog_->check_odd_pages));
			bool const even_pages = static_cast<bool>(fl_get_button(dialog_->check_even_pages));
			bool const odd_only = odd_pages && !even_pages;
			bool const even_only = even_pages && !odd_pages;
			bool const from_is_odd = static_cast<bool>(strToInt(from) % 2);
			if ( (from_is_odd && even_only) || (!from_is_odd && odd_only) ) {
				activate = ButtonPolicy::SMI_INVALID;
			}
		}
	}

	// number of copies only used when output goes to printer
	bool const enable_counter = static_cast<bool>(fl_get_button(dialog_->radio_printer));
	setEnabled(dialog_->counter_copies, enable_counter);

	// sorting only used when printing more than one copy
	bool const enable_sorted = enable_counter && fl_get_counter_value(dialog_->counter_copies) > 1;
	setEnabled(dialog_->check_sorted_copies, enable_sorted);

	return activate;
}
