// -*- C++ -*-
/* form_document.h
 * FormDocument Interface Class
 */

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#include "DialogBase.h"
#include "support/utility.hpp"
#include <vector>

#ifdef __GNUG_
#pragma interface
#endif

class LyXView;
class Dialogs;
class Combox;

struct FD_form_tabbed_document;
struct FD_form_doc_paper;
struct FD_form_doc_class;
struct FD_form_doc_language;
struct FD_form_doc_options;
struct FD_form_doc_bullet;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

/** This class provides an XForms implementation of the FormDocument Popup.
    The table-layout-form here changes values for latex-tabulars
 */
class FormDocument : public DialogBase, public noncopyable {
public:
    /**@name Constructors and Destructors */
    //@{
    /// #FormDocument x(Communicator ..., Popups ...);#
    FormDocument(LyXView *, Dialogs *);
    ///
    ~FormDocument();
    void SetDocumentClassChoice(vector<string> const & choices);
    //@}

    /**@name Real per-instance Callback Methods */
    //@{
    static  int WMHideCB(FL_FORM *, void *);
    static void OKCB(FL_OBJECT *, long);
    static void CancelCB(FL_OBJECT *, long);
    static void ApplyCB(FL_OBJECT *, long);
    static void InputCB(FL_OBJECT *, long);
    //@}

    enum EnumPopupStatus {
        POPUP_UNMODIFIED,
        POPUP_MODIFIED,
        POPUP_READONLY
    };

private:
    void DeactivateDocumentButtons();
    void ActivateDocumentButtons();
    void EnableDocumentLayout();
    void DisableDocumentLayout();
    void CheckDocumentInput(FL_OBJECT * ob, long);
    void ChoiceBulletSize(FL_OBJECT * ob, long);
    void InputBulletLaTeX(FL_OBJECT * ob, long);
    void BulletDepth(FL_OBJECT * ob, long);
    void BulletPanel(FL_OBJECT * ob, long);
    void BulletBMTable(FL_OBJECT * ob, long);
    void checkMarginValues();

    /**@name Real per-instance Callback Methods */
    //@{
    static void DocumentInputCB(FL_OBJECT * ob, long);
    static void ChoiceBulletSizeCB(FL_OBJECT * ob, long);
    static void InputBulletLaTeXCB(FL_OBJECT * ob, long);
    static void BulletDepthCB(FL_OBJECT * ob, long);
    static void BulletPanelCB(FL_OBJECT * ob, long);
    static void BulletBMTableCB(FL_OBJECT * ob, long);
    //@}
	
    /**@name Slot Methods */
    //@{
    /// Create the popup if necessary, update it and display it.
    void show();
    /// Hide the popup.
    void hide();
    /// Update the popup.
    void update();
    void paper_update();
    void class_update();
    void language_update();
    void options_update();
    void bullets_update();
    /// Apply from popup
    void apply();
    void paper_apply();
    bool class_apply();
    bool language_apply();
    bool options_apply();
    void bullets_apply();
    /// Cancel from popup
    void cancel();
    //@}
    /// Build the popup
    void build();
    ///
    /// Explicitly free the popup.
    void free();

    /// Typedefinitions from the fdesign produced Header file
    FD_form_tabbed_document * build_tabbed_document();
    FD_form_doc_paper * build_doc_paper();
    FD_form_doc_class * build_doc_class();
    FD_form_doc_language * build_doc_language();
    FD_form_doc_options * build_doc_options();
    FD_form_doc_bullet * build_doc_bullet();

    /**@name Private Data */
    //@{
    /// Real GUI implementation.
    FD_form_tabbed_document * dialog_;
    FD_form_doc_paper       * paper_;
    FD_form_doc_class       * class_;
    FD_form_doc_language    * language_;
    FD_form_doc_options     * options_;
    FD_form_doc_bullet      * bullets_;
    /// Which LyXView do we belong to?
    LyXView * lv_;
    Dialogs * d_;
    /// Update connection.
    Connection u_;
    /// Hide connection.
    Connection h_;
    /// has form contents changed? Used to control OK/Apply
    EnumPopupStatus status;
    //@}
    int
        ActCell, Confirmed,
	current_bullet_panel, current_bullet_depth;
    ///
    FL_OBJECT * fbullet;
    ///
    Combox * combo_language;
};

#endif
