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

#ifndef FORMCITATION_H
#define FORMCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
#include "LString.h"
#include "support/utility.hpp"
#include "insets/insetcommand.h"

#include <gtk--/container.h>
#include <gtk--/clist.h>
#include <gnome--/entry.h>
#include <gnome--/less.h>
#include <gtk--/button.h>
#include <gtk--/paned.h>
#include <gtk--/box.h>
#include <gtk--/checkbutton.h>

/** This class provides an Gnome implementation of the FormCitation Dialog.
 */
class FormCitation : public DialogBase, public noncopyable {
public:
  ///
  FormCitation(LyXView *, Dialogs *);
  ///
  ~FormCitation();
private:
  /// Slot launching dialog to (possibly) create a new inset
  void createInset( string const & );
  /// Slot launching dialog to an existing inset
  void showInset( InsetCommand * const );
  
  /// Update dialog before showing it
  virtual void update();
  /// Apply from dialog (modify or create inset)
  virtual void apply();
  /// Apply from dialog (modify or create inset)
  virtual void applySelect();
  /// Explicitly free the dialog.
  void free();
  /// Hide the dialog.
  void hide();
  /// Create the dialog if necessary, update it and display it. Not used in this dialog
  void show() { }
  /// Ask user for regexp or keyword(s)
  void showStageSearch();
  /// Ask user to select the citation in the list
  void showStageSelect();
  /// moves from Search to Select "stage"
  void moveFromSearchToSelect();
  
  /// sort biblist
  void sortBibList(gint);
  /// update state of the buttons
  void updateButtons();
  /// clist selection/unselection callback
  void selection_toggled(gint            row,
			 gint            column,
			 GdkEvent       *event,
			 bool selected,
			 bool citeselected);
  /// adds new citation key
  void newCitation();
  /// removes selected citation key
  void removeCitation();
  /// moves citation up
  void moveCitationUp();
  /// moves citation up
  void moveCitationDown();
  /// searches for entries
  void search();
  void searchReg();
  void searchSimple();

  /// add item to the list 
  void addItemToBibList(int i);

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
  Gtk::Button * b_ok;
  Gtk::Button * b_cancel;

  Gnome::Entry * search_text_;
  string search_string_;
  bool use_regexp_;

  Gnome::Less * info_;
  Gnome::Entry * text_after_;

  Gtk::Button * button_select_;
  Gtk::Button * button_unselect_;
  Gtk::Button * button_up_;
  Gtk::Button * button_down_;
  Gtk::CheckButton * button_regexp_;
  
  Gtk::CList * clist_selected_;
  Gtk::CList * clist_bib_;
  
  Gtk::Paned * paned_info_;
  Gtk::Paned * paned_key_;
  Gtk::Box * box_keys_;
  
  std::vector<string> bibkeys;
  ///
  std::vector<string> bibkeysInfo;
};

#endif
