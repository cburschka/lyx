/* FormPrint.C
 * FormPrint Interface Class Implementation
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */


#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormPrint.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "PrinterParams.h"
#include "Liason.h"
#include "debug.h"
#include "BufferView.h"
#include "lyx_gui_misc.h"
#include "gettext.h"

extern "C" {
#include "diaprint_interface.h"
#include "support.h"
}

#include <gtk--/base.h>
#include <gtk--/button.h>

using Liason::printBuffer;
using Liason::getPrinterParams;

FormPrint::FormPrint(LyXView * lv, Dialogs * d)
  : dialog_(0), lv_(lv), d_(d), u_(0), h_(0)
{
  // let the dialog be shown
  // This is a permanent connection so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showPrint.connect(SigC::slot(this,&FormPrint::show));
}


FormPrint::~FormPrint()
{
  hide();
}


void FormPrint::show()
{
  Gtk::Button * b_ok;
  Gtk::Button * b_cancel;
  if (!dialog_)
    {
      GtkWidget * pd = create_DiaPrint();

      dialog_ = Gtk::wrap(pd);
      print_all_  = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "print_all") ) );
      print_odd_  = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "print_odd") ) );
      print_even_ = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "print_even") ) );

      print_pages_ = Gtk::wrap( GTK_CHECK_BUTTON( lookup_widget(pd, "print_pages") ) );

      print_from_ = Gtk::wrap( GTK_SPIN_BUTTON( lookup_widget(pd, "print_from") ) );
      print_to_ = Gtk::wrap( GTK_SPIN_BUTTON( lookup_widget(pd, "print_to") ) );

      order_normal_ = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "order_normal") ) );
      order_reverse_ = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "order_reverse") ) );

      copies_unsorted_ = Gtk::wrap( GTK_CHECK_BUTTON( lookup_widget(pd, "copies_unsorted") ) );
      copies_count_ = Gtk::wrap( GTK_SPIN_BUTTON( lookup_widget(pd, "copies_count") ) );

      printto_printer_ = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "printto_printer") ) );
      printto_file_ = Gtk::wrap( GTK_RADIO_BUTTON( lookup_widget(pd, "printto_file") ) );

      printto_printcommand_ = Gtk::wrap( GNOME_ENTRY( lookup_widget(pd, "printto_printcommand") ) );
      printto_fileentry_ = Gtk::wrap( GNOME_FILE_ENTRY( lookup_widget(pd, "printto_fileentry") ) );

      b_ok = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_print") ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_cancel") ) );

      // setting up connections
      b_ok->clicked.connect(SigC::slot(this, &FormPrint::apply));
      b_ok->clicked.connect(dialog_->destroy.slot());
      b_cancel->clicked.connect(dialog_->destroy.slot());
      dialog_->destroy.connect(SigC::slot(this, &FormPrint::free));

      u_ = d_->updateBufferDependent.connect(SigC::slot(this,
						  &FormPrint::updateSlot));
      h_ = d_->hideBufferDependent.connect(dialog_->destroy.slot());

      if (!dialog_->is_visible()) dialog_->show_all();

      updateSlot();  // make sure its up-to-date
    }
  else
    {
      Gdk_Window dialog_win(dialog_->get_window());
      dialog_win.raise();
    }
}

void FormPrint::hide()
{
  if (dialog_!=0) dialog_->destroy();
}

void FormPrint::free()
{
  if (dialog_!=0)
    {
      dialog_ = 0;
      u_.disconnect();
      h_.disconnect();
    }
}


void FormPrint::apply()
{
  if (!lv_->view()->available()) return;

  PrinterParams::WhichPages wp(PrinterParams::ALL);
  if (print_odd_->get_active()) wp = PrinterParams::ODD;
  else if (print_even_->get_active()) wp = PrinterParams::EVEN;

  string from;
  int to(0);
  if (print_pages_->get_active())
    {
      from = print_from_->get_text();
      to = print_to_->get_value_as_int();
    }
  
  PrinterParams::Target t(PrinterParams::PRINTER);
  if (printto_file_->get_active()) t = PrinterParams::FILE;
  
  // we really should use the return value here I think.
  if (!printBuffer(lv_->buffer(),
		   PrinterParams(t,
				 printto_printcommand_->get_entry()->get_text(),
				 printto_fileentry_->get_full_path(false),
				 wp, from, to,
				 order_reverse_->get_active(),
				 copies_unsorted_->get_active(),
				 copies_count_->get_value_as_int())))
    {
      Alert::alert(_("Error:"),
		 _("Unable to print"),
		 _("Check that your parameters are correct"));
    }
}


// we can safely ignore the parameter because we can always update
void FormPrint::updateSlot(bool)
{
  if (dialog_ != 0 &&
      lv_->view()->available())
    {
      PrinterParams pp(getPrinterParams(lv_->buffer()));

      printto_printcommand_->get_entry()->set_text( pp.printer_name.c_str() );
      ((Gtk::Entry *)printto_fileentry_->gtk_entry())->set_text(pp.file_name.c_str());

      if (pp.target == PrinterParams::PRINTER) printto_printer_->set_active(true);
      else printto_file_->set_active(true);

      if (pp.reverse_order) order_reverse_->set_active(true);
      else order_normal_->set_active(true);

      switch (pp.which_pages)
	{
	case PrinterParams::ODD:
	  print_odd_->set_active(true);
	  break;
	  
	case PrinterParams::EVEN:
	  print_even_->set_active(true);
	  break;
	  
	case PrinterParams::ALL:
	default:
	  print_all_->set_active(true);
	  break;
	}
      
      // hmmm... maybe a bit weird but maybe not
      // we might just be remembering the last
      // time this was printed.
      if (!pp.from_page.empty())
	{
	  print_to_->set_value(pp.to_page);
	  print_from_->set_value(strToInt(pp.from_page));
	}
    }
}
