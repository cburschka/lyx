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

#include <vector>
#include <boost/smart_ptr.hpp>

#ifdef __GNUG_
#pragma interface
#endif

#include "FormBaseDeprecated.h"

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
    @author Jürgen Vigna
 */
class FormDocument : public FormBaseBD {
public:
	/// #FormDocument x(Communicator ..., Popups ...);#
	FormDocument(LyXView *, Dialogs *);
	///
	static void ComboInputCB(int, void *, Combox *);
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
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( FL_OBJECT *, long );
	/// Update the popup.
	virtual void update();
	/// Apply from popup
	virtual void apply();
	/// Cancel from popup
	virtual void cancel();

	///
	virtual FL_FORM * form() const;

	///
	bool CheckDocumentInput(FL_OBJECT * ob, long);
	///
	void ChoiceBulletSize(FL_OBJECT * ob, long);
	///
	void InputBulletLaTeX(FL_OBJECT * ob, long);
	///
	void BulletDepth(FL_OBJECT * ob, State);
	///
	void BulletPanel(FL_OBJECT * ob, State);
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
	boost::scoped_ptr<FD_form_tabbed_document> dialog_;
	///
	boost::scoped_ptr<FD_form_doc_paper>       paper_;
	///
	boost::scoped_ptr<FD_form_doc_class>       class_;
	///
	boost::scoped_ptr<FD_form_doc_language>    language_;
	///
	boost::scoped_ptr<FD_form_doc_options>     options_;
	///
	boost::scoped_ptr<FD_form_doc_bullet>      bullets_;
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
	boost::scoped_ptr<Combox> combo_language;
	///
	boost::scoped_ptr<Combox> combo_doc_class;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormDocument::bc()
{
	return bc_;
}

#endif
