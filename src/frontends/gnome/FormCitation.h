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
#include "boost/utility.hpp"
#include "insets/insetcommand.h"

#include <gtk--/container.h>
#include <gtk--/clist.h>
#include <gnome--/entry.h>
#include <gnome--/less.h>
#include <gtk--/button.h>
#include <gtk--/paned.h>
#include <gtk--/box.h>
#include <gtk--/checkbutton.h>
#include "pixbutton.h"

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

  /// The following three methods do nothing in this implementation
  virtual void update() { }
  virtual void apply() { }
  void show() { }

  /// Explicitly free the dialog.
  void free();
  /// Hide the dialog.
  void hide();

  /// Apply from dialog (modify or create inset)
  void applySelect();
  /// Apply from dialog (modify or create inset)
  void applyEdit();

  /// Ask user for requested action: add new citation or edit the existing ones
  void showStageAction();
  /// Ask user for regexp or keyword(s)
  void showStageSearch();
  /// Ask user to select the citation in the list
  void showStageSelect();
  /// Ask user to edit the citation in the list
  void showStageEdit();

  /// moves from Search to Select "stage"
  void moveFromSearchToSelect();
  /// moves from Action to Search "stage"
  void moveFromActionToSearch();
  /// moves from Action to Edit "stage"
  void moveFromActionToEdit();
  
  /// sort biblist
  void sortBibList(gint);
  /// update state of the buttons
  void updateButtons();
  /// clist selection/unselection callback
  void selectionToggled(gint            row,
			gint            column,
			GdkEvent       *event,
			bool selected,
			bool citeselected);

  /// removes selected citation key
  void removeCitation();
  /// moves citation up
  void moveCitationUp();
  /// moves citation up
  void moveCitationDown();

  /// searches for entries (calls searchReg or searchSimple)
  void search();
  /// searches for entries using regexp
  void searchReg();
  /// searches for entries containing keyword(s)
  void searchSimple();

  /// adds item to clist_bib_
  void addItemToBibList(int i);

  /// sets all widget pointers to NULL
  void cleanupWidgets();
  /// initializes all non-NULL member widgets
  void initWidgets();
  /// stores configuration of all non-NULL member widgets
  void storeWidgets();
  
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

  Gnome::PixButton * button_unselect_;
  Gnome::PixButton * button_up_;
  Gnome::PixButton * button_down_;
  Gtk::CheckButton * button_regexp_;
  
  Gtk::CList * clist_selected_;
  Gtk::CList * clist_bib_;
  
  Gtk::Paned * paned_info_;
  
  std::vector<string> bibkeys;
  ///
  std::vector<string> bibkeysInfo;
};

#endif
