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
#include "xform_macros.h"
#include "input_validators.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "PrinterParams.h"
#include "Liason.h"
#include "debug.h"
#include "BufferView.h"
#include "lyx_gui_misc.h"
#include "gettext.h"


#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

#ifdef CXX_WORKING_NAMESPACES
using Liason::printBuffer;
using Liason::getPrinterParams;
#endif

C_RETURNCB(FormPrint,  WMHideCB)
C_GENERICCB(FormPrint, OKCB)
C_GENERICCB(FormPrint, ApplyCB)
C_GENERICCB(FormPrint, CancelCB)
C_GENERICCB(FormPrint, InputCB)


FormPrint::FormPrint(LyXView * lv, Dialogs * d)
	: dialog_(0), lv_(lv), d_(d), u_(0), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPrint.connect(slot(this,&FormPrint::show));
}


FormPrint::~FormPrint()
{
	free();
}


void FormPrint::build()
{
	dialog_ = build_print();
}


void FormPrint::show()
{
	if (!dialog_) {
		build();
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
		
		fl_set_form_atclose(dialog_->form_print,
				    C_FormPrintWMHideCB, 0);
	}

	update();  // make sure its up-to-date

	if (dialog_->form_print->visible) {
		fl_raise_form(dialog_->form_print);
	} else {
		fl_show_form(dialog_->form_print,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_FULLBORDER,
			     _("Print"));
		u_ = d_->updateBufferDependent.connect(slot(this,
							    &FormPrint::update));
		h_ = d_->hideBufferDependent.connect(slot(this,
							  &FormPrint::hide));
	}
}


void FormPrint::hide()
{
	if (dialog_
	    && dialog_->form_print
	    && dialog_->form_print->visible) {
		fl_hide_form(dialog_->form_print);
		u_.disconnect();
		h_.disconnect();
	}
}


void FormPrint::apply()
{
	if (!lv_->view()->available()) {
		return;
	}

	PrinterParams::WhichPages wp(PrinterParams::ALL);
	if (fl_get_button(dialog_->radio_even_pages)) {
		wp = PrinterParams::EVEN;
	} else if (fl_get_button(dialog_->radio_odd_pages)) {
		wp = PrinterParams::ODD;
	}

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

	PrinterParams::Target t(PrinterParams::PRINTER);
	if (fl_get_button(dialog_->radio_file)) {
		t = PrinterParams::FILE;
	}

	// we really should use the return value here I think.
	if (!printBuffer(lv_->buffer(),
			 PrinterParams(t,
				       string(fl_get_input(dialog_->input_printer)),
				       string(fl_get_input(dialog_->input_file)),
				       wp, from, to,
				       static_cast<bool>(fl_get_button(dialog_->
								       radio_order_reverse)),
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

		switch (pp.target) {
		case PrinterParams::FILE:
			fl_set_button(dialog_->radio_printer, 0);
			fl_set_button(dialog_->radio_file, 1);
			break;

		case PrinterParams::PRINTER:
		default:
			fl_set_button(dialog_->radio_printer, 1);
			fl_set_button(dialog_->radio_file, 0);
			break;
		}

		switch (pp.reverse_order) {
		case true:
			fl_set_button(dialog_->radio_order_normal, 0);
			fl_set_button(dialog_->radio_order_reverse, 1);
			break;

		case false:
		default:
			fl_set_button(dialog_->radio_order_normal, 1);
			fl_set_button(dialog_->radio_order_reverse, 0);
			break;
		}
// should be able to remove the various set_button 0 and rely on radio button
// action.  Provided xforms is smart enough :D
		fl_set_button(dialog_->radio_all_pages, 0);
		fl_set_button(dialog_->radio_odd_pages, 0);
		fl_set_button(dialog_->radio_even_pages, 0);
		switch (pp.which_pages) {
		case PrinterParams::ODD:
			fl_set_button(dialog_->radio_odd_pages, 1);
			break;

		case PrinterParams::EVEN:
			fl_set_button(dialog_->radio_even_pages, 1);
			break;

		case PrinterParams::ALL:
		default:
			fl_set_button(dialog_->radio_all_pages, 1);
			break;
		}

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

		// Even readonly docs can be printed
		// these 4 activations are probably superfluous but I'm
		// being explicit for a reason.
		// They can probably be removed soon along with a few more
		// of the de/activations above once input() is a bit smarter.
		fl_activate_object(dialog_->input_count);
		fl_activate_object(dialog_->input_file);
		fl_activate_object(dialog_->input_from_page);
		fl_activate_object(dialog_->input_printer);
		// and we should always be in a working state upon exit
		input();
	}
}


// It would be nice if we checked for cases like:
// Print only-odd-pages and from_page == an even number
//
void FormPrint::input()
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

	if (activate) {
		fl_activate_object(dialog_->button_ok);
		fl_activate_object(dialog_->button_apply);
		fl_set_object_lcol(dialog_->button_ok, FL_BLACK);
		fl_set_object_lcol(dialog_->button_apply, FL_BLACK);
	} else {
		fl_deactivate_object(dialog_->button_ok);
		fl_deactivate_object(dialog_->button_apply);
		fl_set_object_lcol(dialog_->button_ok, FL_INACTIVE);
		fl_set_object_lcol(dialog_->button_apply, FL_INACTIVE);
	}
}


void FormPrint::free()
{
	// we don't need to delete u and h here because
	// hide() does that after disconnecting.
	if (dialog_) {
		if (dialog_->form_print
		    && dialog_->form_print->visible) {
			hide();
		}
		fl_free_form(dialog_->form_print);
		delete dialog_;
		dialog_ = 0;
	}
}


int FormPrint::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormPrint * pre = static_cast<FormPrint*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormPrint::OKCB(FL_OBJECT * ob, long)
{
	FormPrint * pre = static_cast<FormPrint*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();
}


void FormPrint::ApplyCB(FL_OBJECT * ob, long)
{
	FormPrint * pre = static_cast<FormPrint*>(ob->form->u_vdata);
	pre->apply();
}


void FormPrint::CancelCB(FL_OBJECT * ob, long)
{
	FormPrint * pre = static_cast<FormPrint*>(ob->form->u_vdata);
	pre->hide();
}


void FormPrint::InputCB(FL_OBJECT * ob, long)
{
	FormPrint * pre = static_cast<FormPrint*>(ob->form->u_vdata);
	pre->input();
}
