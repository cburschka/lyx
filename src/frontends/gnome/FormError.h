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

#ifndef FORMERROR_H
#define FORMERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
#include "LString.h"
#include "insets/inseterror.h"

#include <gtk--/container.h>

/** This class provides an Gnome implementation of the FormError Dialog.
 */
class FormError : public DialogBase {
public:
  ///
  FormError(LyXView *, Dialogs *);
  ///
  ~FormError();
private:
  /// Slot launching dialog to an existing inset
  void showInset( InsetError * const );
  
  /// Update dialog before showing it
  virtual void update() { }
  virtual void updateSlot(bool = false);
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
  InsetError * inset_;
  /// Update connection.
  Connection u_;
  /// Hide connection.
  Connection h_;
  /// inset::hide connection.
  Connection ih_;

  /// Real GUI implementation.
  Gtk::Container * dialog_;
};

#endif
