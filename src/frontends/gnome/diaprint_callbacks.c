/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

/* Print dialog callbacks
 * Controls the state of the widgets
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "diaprint_callbacks.h"
#include "diaprint_interface.h"
#include "support.h"


void
diaprint_set_printdest_state( GtkWidget *wid, gboolean active_is_printer )
{
  if (active_is_printer)
    {
      gtk_widget_set_sensitive(lookup_widget(wid, "printto_printcommand"), TRUE);
      gtk_widget_set_sensitive(lookup_widget(wid, "printto_fileentry"), FALSE);  
    }
  else
    {
      gtk_widget_set_sensitive(lookup_widget(wid, "printto_printcommand"), FALSE);
      gtk_widget_set_sensitive(lookup_widget(wid, "printto_fileentry"), TRUE);  
    }
}

void
diaprint_set_pages_state( GtkWidget *wid, gboolean active_pages )
{
  gtk_widget_set_sensitive(lookup_widget(wid, "print_from"), active_pages);
  gtk_widget_set_sensitive(lookup_widget(wid, "print_to"), active_pages);  
}

void
diaprint_on_diaprint_show              (GtkWidget       *widget,
                                        gpointer         user_data)
{
  diaprint_set_printdest_state(widget,
			       gtk_toggle_button_get_active
			       (GTK_TOGGLE_BUTTON(lookup_widget(widget, "printto_printer")))
			       );
  diaprint_set_pages_state(widget, 
			   gtk_toggle_button_get_active
			   (GTK_TOGGLE_BUTTON(lookup_widget(widget, "print_pages")))
			   );
}

void
diaprint_on_printto_file_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  diaprint_set_printdest_state(GTK_WIDGET(togglebutton), FALSE);
}


void
diaprint_on_printto_printer_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  diaprint_set_printdest_state(GTK_WIDGET(togglebutton), TRUE);
}

void
diaprint_on_print_pages_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  diaprint_set_pages_state(GTK_WIDGET(togglebutton),
			   gtk_toggle_button_get_active(togglebutton));
}

void
diaprint_on_print_from_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int nmin = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(editable) );
  GtkSpinButton * to = GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(editable), "print_to"));
  GtkAdjustment * a = gtk_spin_button_get_adjustment(to);
  int v = gtk_spin_button_get_value_as_int(to);

  a->lower = nmin;
  if (v < nmin)
    gtk_spin_button_set_value(to, nmin);
  else
    gtk_spin_button_set_value(to, v);    
}


