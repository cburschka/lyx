// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *	    This file copyright 1999-2000
 *	    Allan Rae
 *======================================================*/
/* FormTabular.h
 * FormTabular Interface Class
 */

#ifndef FORMTABULAR_H
#define FORMTABULAR_H

#include "DialogBase.h"
#include "support/utility.hpp"

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

class LyXView;
class Dialogs;
class InsetTabular;
struct FD_form_tabular;
struct FD_form_tabular_options;
struct FD_form_column_options;
struct FD_form_cell_options;
struct FD_form_longtable_options;
struct FD_form_create_tabular;

/** This class provides an XForms implementation of the FormTabular Dialog.
    The tabular dialog allows users to set/save their tabular.
 */
class FormTabular : public DialogBase, public noncopyable {
public:
    /**@name Constructors and Destructors */
    //@{
    /// #FormTabular x(LyXFunc ..., Dialogs ...);#
    FormTabular(LyXView *, Dialogs *);
    ///
    ~FormTabular();
    //@}

    /**@name Real per-instance Callback Methods */
    //@{
    static  int WMHideCB(FL_FORM *, void *);
    static void CloseCB(FL_OBJECT *, long);
    static void OKCB(FL_OBJECT *, long);
    static void CancelCB(FL_OBJECT *, long);
    static void ApplyCB(FL_OBJECT *, long);
    static void InputCB(FL_OBJECT *, long);
    //@}

private:
    /**@name Slot Methods */
    //@{
    /// Create the dialog if necessary, update it and display it.
    void show();
    void show_create();
    void showInset(InsetTabular *);
    /// Hide the dialog.
    void hide();
    void hide_create();
    void hideInset(InsetTabular *);
    /// Update the dialog.
    void update();
    void updateInset(InsetTabular *);
    bool local_update(bool);
    //@}
    
    /**@name Dialog internal methods */
    //@{
    /// Apply from dialog
    void apply_create();
    /// Build the dialog
    void build();
    ///
    void SetTabularOptions(FL_OBJECT *, long);
    ///
    FD_form_tabular * build_tabular();
    ///
    FD_form_tabular_options * build_tabular_options();
    ///
    FD_form_column_options * build_column_options();
    ///
    FD_form_cell_options * build_cell_options();
    ///
    FD_form_longtable_options * build_longtable_options();
    ///
    FD_form_create_tabular * build_create_tabular();
    //@}

    /**@name Private Data */
    //@{
    /// Real GUI implementation.
    FD_form_tabular * dialog_;
    ///
    FD_form_tabular_options * tabular_options_;
    ///
    FD_form_column_options * column_options_;
    ///
    FD_form_cell_options * cell_options_;
    ///
    FD_form_longtable_options * longtable_options_;
    ///
    FD_form_create_tabular * create_tabular_;
    //
    /// Which LyXView do we belong to?
    LyXView * lv_;
    ///
    Dialogs * d_;
    /// Update connection.
    Connection u_;
    /// Hide connection.
    Connection h_;
    ///
    InsetTabular * inset_;
    ///
    int actCell_;
    //@}
};

#endif
