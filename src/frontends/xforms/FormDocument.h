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

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#include "FormBase.h"
#include <vector>

#ifdef __GNUG_
#pragma interface
#endif

class LyXView;
class Dialogs;
class Combox;
class BufferParams;

struct FD_form_tabbed_document;
struct FD_form_doc_paper;
struct FD_form_doc_class;
struct FD_form_doc_language;
struct FD_form_doc_options;
struct FD_form_doc_bullet;

/** This class provides an XForms implementation of the FormDocument Popup.
    The table-layout-form here changes values for latex-tabulars
 */
class FormDocument : public FormBase {
public:
    /// #FormDocument x(Communicator ..., Popups ...);#
    FormDocument(LyXView *, Dialogs *);
    ///
    ~FormDocument();
    /// this operates very differently to FormBase::InputCB
    static void InputCB(FL_OBJECT *, long);
    ///
    static void ComboInputCB(int, void *, Combox *);
    ///
    static void ChoiceClassCB(FL_OBJECT *, long);
    ///
    static void ChoiceBulletSizeCB(FL_OBJECT * ob, long);
    ///
    static void InputBulletLaTeXCB(FL_OBJECT * ob, long);
    ///
    static void BulletDepthCB(FL_OBJECT * ob, long);
    ///
    static void BulletPanelCB(FL_OBJECT * ob, long);
    ///
    static void BulletBMTableCB(FL_OBJECT * ob, long);

private:
    ///
    bool CheckDocumentInput(FL_OBJECT * ob, long);
    ///
    void ChoiceBulletSize(FL_OBJECT * ob, long);
    ///
    void InputBulletLaTeX(FL_OBJECT * ob, long);
    ///
    void BulletDepth(FL_OBJECT * ob, long);
    ///
    void BulletPanel(FL_OBJECT * ob, long);
    ///
    void BulletBMTable(FL_OBJECT * ob, long);
    ///
    void checkMarginValues();
    ///
    void checkReadOnly();
    ///
    void CheckChoiceClass(FL_OBJECT * ob, long);
    ///
    void UpdateLayoutDocument(BufferParams const & params);

    /// Update the popup.
    virtual void update();
    ///
    void paper_update(BufferParams const &);
    ///
    void class_update(BufferParams const &);
    ///
    void language_update(BufferParams const &);
    ///
    void options_update(BufferParams const &);
    ///
    void bullets_update(BufferParams const &);
    /// Apply from popup
    virtual void apply();
    ///
    void paper_apply();
    ///
    bool class_apply();
    ///
    bool language_apply();
    ///
    bool options_apply();
    ///
    void bullets_apply();
    /// Cancel from popup
    virtual void cancel();
    ///
    virtual void restore() {
	update();
    }
    /// Build the popup
    virtual void build();
    ///
    virtual FL_FORM * const FormDocument::form() const;

    /// Typedefinitions from the fdesign produced Header file
    FD_form_tabbed_document * build_tabbed_document();
    ///
    FD_form_doc_paper * build_doc_paper();
    ///
    FD_form_doc_class * build_doc_class();
    ///
    FD_form_doc_language * build_doc_language();
    ///
    FD_form_doc_options * build_doc_options();
    ///
    FD_form_doc_bullet * build_doc_bullet();

    /// Real GUI implementation.
    FD_form_tabbed_document * dialog_;
    ///
    FD_form_doc_paper       * paper_;
    ///
    FD_form_doc_class       * class_;
    ///
    FD_form_doc_language    * language_;
    ///
    FD_form_doc_options     * options_;
    ///
    FD_form_doc_bullet      * bullets_;
    ///
    int ActCell;
    ///
    int Confirmed;
    ///
    int current_bullet_panel;
    ///
    int current_bullet_depth;
    ///
    FL_OBJECT * fbullet;
    ///
    Combox * combo_language;
    ///
    Combox * combo_doc_class;
};

#endif
