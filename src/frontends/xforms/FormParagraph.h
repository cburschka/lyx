// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright (C) 2000 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 *======================================================*/

#ifndef FORM_PARAGRAPH_H
#define FORM_PARAGRAPH_H

#include "DialogBase.h"
#include "support/utility.hpp"
#include <vector>

#ifdef __GNUG_
#pragma interface
#endif

class LyXView;
class Dialogs;
class NoRepeatedApplyReadOnlyPolicy;
//template <class x> class ButtonController;
class ButtonController;

struct FD_form_tabbed_paragraph;
struct FD_form_paragraph_general;
struct FD_form_paragraph_extra;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

/** This class provides an XForms implementation of the FormParagraph Popup.
    The table-layout-form here changes values for latex-tabulars
 */
class FormParagraph : public DialogBase, public noncopyable {
public:
    FormParagraph(LyXView *, Dialogs *);
    ~FormParagraph();
    ///
    static  int WMHideCB(FL_FORM *, void *);
    static void OKCB(FL_OBJECT *, long);
    static void ApplyCB(FL_OBJECT *, long);
    static void CancelCB(FL_OBJECT *, long);
    static void RestoreCB(FL_OBJECT *, long);
    static void InputCB(FL_OBJECT *, long);
    static void VSpaceCB(FL_OBJECT *, long);
    ///
    enum EnumPopupStatus {
	///
        POPUP_UNMODIFIED,
	///
        POPUP_MODIFIED,
	///
        POPUP_READONLY
    };

private:
    /// Create the popup if necessary, update it and display it.
    void show();
    /// Hide the popup.
    void hide();
    /// Update the popup.
    void update();
    ///
    void general_update();
    ///
    void extra_update();
    /// Apply from popup
    void apply();
    ///
    void general_apply();
    ///
    void extra_apply();
    /// Cancel from popup
    void cancel();
    /// Build the popup
    void build();
    /// Explicitly free the popup.
    void free();
    ///
    bool CheckParagraphInput(FL_OBJECT * ob, long);


    /// Typedefinitions from the fdesign produced Header file
    FD_form_tabbed_paragraph * build_tabbed_paragraph();
    ///
    FD_form_paragraph_general * build_paragraph_general();
    ///
    FD_form_paragraph_extra * build_paragraph_extra();

    /// Real GUI implementation.
    FD_form_tabbed_paragraph  * dialog_;
    ///
    FD_form_paragraph_general * general_;
    ///
    FD_form_paragraph_extra   * extra_;
    /// Which LyXView do we belong to?
    LyXView * lv_;
    ///
    Dialogs * d_;
    /// Update connection.
    Connection u_;
    /// Hide connection.
    Connection h_;
    /// has form contents changed? Used to control OK/Apply
    EnumPopupStatus status;
    ///
	ButtonController/*<NoRepeatedApplyReadOnlyPolicy>*/ * bc_;
};

#endif
