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

#ifndef FORMTOC_H
#define FORMTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"
#include "buffer.h"

#include <gnome--/dialog.h>
#include <gtk--/optionmenu.h>
#include <gtk--/list.h>

/** This class provides an Gnome implementation of the FormToc Dialog.
 */
class FormToc : public DialogBase, public noncopyable {
public:
  ///
  FormToc(LyXView *, Dialogs *);
  ///
  ~FormToc();
private:
  /// Slot launching dialog to (possibly) create a new inset
  void createInset( string const & );
  /// Slot launching dialog to an existing inset
  void showInset( InsetCommand * const );
  
  /// Update dialog before showing it
  virtual void update(bool = false);
  /// Explicitly free the dialog.
  void free();
  /// Create the dialog if necessary, update it and display it.
  void show();
  /// Hide the dialog.
  void hide();
  /// move cursor in LyXView
  void apply(Buffer::TocItem);
  /// change type of the list
  void changeList(Buffer::TocType);
  
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
  Gnome::Dialog * dialog_;

  Gtk::OptionMenu * choice_;

  Gtk::List * list_;

  bool ignore_callback_;
};

#endif
