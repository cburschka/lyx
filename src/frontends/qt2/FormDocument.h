// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright (C) 2000 The LyX Team.
 *
 *           @author Jürgen Vigna, Kalle Dalheimer
 *
 *======================================================*/

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "FormBase.h"
#include <vector>

#ifdef __GNUG_
#pragma interface
#endif

class LyXView;
class Dialogs;
class Combox;
class BufferParams;

class QWidget;
class FormDocumentDialog;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

class QComboBox;

#include <qpixmap.h>

/** This class provides an Qt2 implementation of the FormDocument Popup.
    The table-layout-form here changes values for latex-tabulars
 */
class FormDocument : public DialogBase {
public:
    /// #FormDocument x(Communicator ..., Popups ...);#
    FormDocument(LyXView *, Dialogs *);
    ///
    ~FormDocument();
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
  /// Show the dialog.
   void show();
   /// Hide the dialog.
   void hide();

    void checkMarginValues();
    ///
    void checkReadOnly();
    ///
    void UpdateLayoutDocument(BufferParams const & params);

public:
    ///
     void checkChoiceClass(QComboBox* cb);
//     ///
     bool checkDocumentInput(QWidget* w);
//     ///
  void bulletDepth( int );
    /// 
 void choiceBulletSize();
//     ///
  void inputBulletLaTeX();
//     ///
  void setBulletPics();
//     ///
  void bulletBMTable( int );

    /// Update the popup.
    void update();
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
    void apply();
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
    void cancel();
    /// Build the popup
    void build();
    /// Explicitly free the popup.
    void free();

//     /// Typedefinitions from the fdesign produced Header file
//     FD_form_tabbed_document * build_tabbed_document();
//     ///
//     FD_form_doc_paper * build_doc_paper();
//     ///
//     FD_form_doc_class * build_doc_class();
//     ///
//     FD_form_doc_language * build_doc_language();
//     ///
//     FD_form_doc_options * build_doc_options();
//     ///
//     FD_form_doc_bullet * build_doc_bullet();

//     /// Real GUI implementation.
  FormDocumentDialog* dialog_;

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
    int ActCell;
    ///
    int Confirmed;
    ///
    int currentBulletPanel;
    ///
    int currentBulletDepth;
    ///

  QPixmap* standardpix;
  QPixmap* amssymbpix;
  QPixmap* psnfss1pix;
  QPixmap* psnfss2pix;
  QPixmap* psnfss3pix;
  QPixmap* psnfss4pix;
  QPixmap bulletpics[36];
};

#endif
