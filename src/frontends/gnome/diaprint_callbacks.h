/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#include <gnome.h>


void
diaprint_on_diaprint_show              (GtkWidget       *widget,
                                        gpointer         user_data);

void
diaprint_on_print_all_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
diaprint_on_print_pages_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
diaprint_on_printto_file_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
diaprint_on_printto_printer_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
diaprint_on_print_from_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

