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

#ifndef FORMREF_H
#define FORMREF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
#include "LString.h"
#include "insets/insetcommand.h"

#include <gtk--/container.h>
#include <gtk--/clist.h>
#include <gtk--/button.h>
#include <gnome--/entry.h>
#include <gtk--/optionmenu.h>
#include <gtk--/menu.h>
#include "pixbutton.h"

/** This class provides an Gnome implementation of the FormRef Dialog.
 */
class FormRef : public DialogBase {
public:
  ///
  FormRef(LyXView *, Dialogs *);
  ///
  ~FormRef();
private:
  ///
  enum Type{
    ///
    REF,
    ///
    PAGEREF,
    ///
    VREF,
    ///
    VPAGEREF,
    ///
    PRETTYREF
  };
  ///
  enum Goto{
    ///
    GOREF,
    ///
    GOBACK
  };
  ///
  enum ActionType {
    ///
    INSERT,
    ///
    EDIT
  };

  /// Slot launching dialog to (possibly) create a new inset
  void createInset( string const & );
  /// Slot launching dialog to an existing inset
  void showInset( InsetCommand * const );
  
  /// Update dialog before showing it (not used in this implementation)
  virtual void update() { } 
  virtual void updateSlot(bool = false);
  /// Apply from dialog (modify or create inset)
  virtual void apply();
  /// Explicitly free the dialog.
  void free();
  /// dummy function
  virtual void show() { }
  /// Show selection of the references
  void showStageSelect();
  /// Edit properties
  void showStageAction();
  /// Error (no labels)
  void showStageError(string const & mess);
  /// Hide the dialog.
  void hide();
  /// Go to reference or return back
  void gotoRef();
  
  /// moves from Search to Select "stage"
  void moveFromSelectToAction();

  /// changes the type of the reference
  void changeType(Type);
  ///
  Type getType() const;
  ///
  string getName( Type type ) const;

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
  /// 
  std::vector<string> refs;
  ///
  Type reftype_;
  ///
  Goto gototype_;
  ///
  ActionType acttype_;
  
  /// Real GUI implementation.
  Gtk::Container * dialog_;
  Gtk::CList * list_;
  Gtk::Button * b_ok;
  Gtk::Button * b_cancel;
  Gnome::Entry * name_;
  Gtk::OptionMenu * choice_;
  Gnome::PixButton * b_goto;
};

#endif
