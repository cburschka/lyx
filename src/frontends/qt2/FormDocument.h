// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright (C) 2000 The LyX Team.
 *
 *           @author Kalle Dalheimer
 *
 *======================================================*/

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#include <vector>
#include <boost/smart_ptr.hpp>

#include "Qt2Base.h"
#include "qt2BC.h"
#undef emit

#ifdef __GNUG_
#pragma interface
#endif

class ControlDocument;
class FormDocumentDialogImpl;
class BufferParams;
class LyXView;
class Dialogs;

/** This class provides an Qt2 implementation of the FormDocument Popup.
    The table-layout-form here changes values for latex-tabulars
    @author Kalle Dalheimer
 */
class FormDocument
  : public Qt2CB<ControlDocument, Qt2DB<FormDocumentDialogImpl> >
{
public:
	/// #FormDocument x(Communicator ..., Popups ...);#
	FormDocument(LyXView *, Dialogs *);
private:
	///
	enum State {
		///
		INPUT,
		///
		CHECKCHOICECLASS,
		///
		CHOICEBULLETSIZE,
		///
		INPUTBULLETLATEX,
		///
		BULLETDEPTH1,
		///
		BULLETDEPTH2,
		///
		BULLETDEPTH3,
		///
		BULLETDEPTH4,
		///
		BULLETPANEL1,
		///
		BULLETPANEL2,
		///
		BULLETPANEL3,
		///
		BULLETPANEL4,
		///
		BULLETPANEL5,
		///
		BULLETPANEL6,
		///
		BULLETBMTABLE
	};
	/// Pointer to the actual instantiation of the ButtonController.
	virtual qt2BC & bc();
	/// Build the dialog
	virtual void build();
// 	/// Filter the inputs
// 	virtual bool input( FL_OBJECT *, long );
	/// Update the popup.
	virtual void update();
	/// Apply from popup
	virtual void apply();
	/// Cancel from popup
	virtual void cancel();

// 	///
// 	virtual QDialog* form() const;

	///
	bool CheckDocumentInput(QWidget* ob, long);
	///
	void ChoiceBulletSize(QWidget* ob, long);
	///
	void InputBulletLaTeX(QWidget* ob, long);
	///
	void BulletDepth(QWidget* ob, State);
	///
	void BulletPanel(QWidget* ob, State);
	///
	void BulletBMTable(QWidget* ob, long);
	///
	void checkMarginValues();
	///
	void checkReadOnly();
	///
	void CheckChoiceClass(QWidget* ob, long);
	///
	void UpdateLayoutDocument(BufferParams const & params);

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

	/// Fdesign generated methods
	QDialog* build_tabbed_document();
	///
	QDialog* build_doc_paper();
	///
	QDialog* build_doc_class();
	///
	QDialog* build_doc_language();
	///
	QDialog* build_doc_options();
	///
	QDialog* build_doc_bullet();

	/// Real GUI implementation.
	boost::scoped_ptr<FormDocumentDialogImpl> dialog_;
	///
	int ActCell;
	///
	int Confirmed;
	///
	int current_bullet_panel;
	///
	int current_bullet_depth;
// 	///
// 	FL_OBJECT * fbullet;
// 	///
// 	boost::scoped_ptr<Combox> combo_language;
// 	///
// 	boost::scoped_ptr<Combox> combo_doc_class;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, qt2BC> bc_;
};


inline
qt2BC & FormDocument::bc()
{
	return bc_;
}

#endif
