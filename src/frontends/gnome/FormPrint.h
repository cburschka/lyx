// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */
/* FormPrint.h
 * FormPrint Interface Class
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include "DialogBase.h"
#include "support/utility.hpp"

#include <gtk--/widget.h>
#include <gtk--/radiobutton.h>
#include <gtk--/checkbutton.h>
#include <gtk--/spinbutton.h>
#include <gnome--/entry.h>
#include <gnome--/file-entry.h>


#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class Dialogs;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

/** This class provides an Gnome implementation of the FormPrint Dialog.
    The print dialog allows users to print their documents.
 */
class FormPrint : public DialogBase, public noncopyable {
public:
  /**@name Constructors and Destructors */
  //@{
  /// #FormPrint x(LyXFunc ..., Dialogs ...);#
  FormPrint(LyXView *, Dialogs *);
  ///
  ~FormPrint();
  //@}
  

private:
  /**@name Slot Methods */
  //@{
  /// Create the dialog if necessary, update it and display it.
  void show();
  /// Hide the dialog.
  void hide();
  /// Update the dialog.
  void update();
  /// Free memory
  void free();
  //@}
  
  /**@name Dialog internal methods */
  //@{
  /// Apply from dialog
  void apply();
  //@}
  
  /**@name Private Data */
  //@{
  /// Real GUI implementation.
  Gtk::Widget * dialog_;
  
  Gtk::RadioButton * print_all_;
  Gtk::RadioButton * print_odd_;
  Gtk::RadioButton * print_even_;

  Gtk::CheckButton * print_pages_;

  Gtk::SpinButton  * print_from_;
  Gtk::SpinButton  * print_to_;
  
  Gtk::RadioButton * order_normal_;
  Gtk::RadioButton * order_reverse_;

  Gtk::CheckButton * copies_unsorted_;
  Gtk::SpinButton  * copies_count_;

  Gtk::RadioButton * printto_printer_;
  Gtk::RadioButton * printto_file_;

  Gnome::Entry * printto_printcommand_;
  Gnome::FileEntry * printto_fileentry_;
  
  /// Which LyXView do we belong to?
  LyXView * lv_;
  Dialogs * d_;
  /// Update connection.
  Connection u_;
  /// Hide connection.
  Connection h_;
  //@}
};

#endif
