// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FORMINDEX_H
#define FORMINDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
#include "LString.h"
#include "support/utility.hpp"
#include "insets/insetcommand.h"

#include <gtk--/container.h>
#include <gnome--/entry.h>
#include <gtk--/label.h>

/** This class provides an Gnome implementation of the FormIndex Dialog.
 */
class FormIndex : public DialogBase, public noncopyable {
public:
  ///
  FormIndex(LyXView *, Dialogs *);
  ///
  ~FormIndex();
private:
  /// Slot launching dialog to (possibly) create a new inset
  void createInset( string const & );
  /// Slot launching dialog to an existing inset
  void showInset( InsetCommand * const );
  
  /// Update dialog before showing it
  virtual void update();
  /// Apply from dialog (modify or create inset)
  virtual void apply();
  /// Explicitly free the dialog.
  void free();
  /// Create the dialog if necessary, update it and display it.
  void show();
  /// Hide the dialog.
  void hide();
  
  /** Which LyXFunc do we use?
      We could modify Dialogs to have a visible LyXFunc* instead and
      save a couple of bytes per dialog.
  */
  LyXView * lv_;
  /** Which Dialogs do we belong to?
      Used so we can get at the signals we have to connect to.
  */
  Dialogs * d_;
  /// pointer to the inset passed through showInset (if any)
  InsetCommand * inset_;
  /// the nitty-griity. What is modified and passed back
  InsetCommandParams params;
  /// Update connection.
  Connection u_;
  /// Hide connection.
  Connection h_;
  /// inset::hide connection.
  Connection ih_;

  /// Real GUI implementation.
  Gtk::Container * dialog_;
  Gnome::Entry * keyword_;
  Gtk::Button * b_ok;
  Gtk::Button * b_cancel;
};

#endif
