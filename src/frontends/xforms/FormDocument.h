// -*- C++ -*-
/**
 * \file FormDocument.h
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

#include <boost/scoped_ptr.hpp>

#include <vector>

class Combox;
class BufferParams;

struct FD_document;
struct FD_document_paper;
struct FD_document_class;
struct FD_document_language;
struct FD_document_options;
struct FD_document_bullet;

/** This class provides an XForms implementation of the FormDocument dialog.
    The table-layout-form here changes values for latex-tabulars
 */
class FormDocument : public FormBaseBD {
public:
	FormDocument(LyXView &, Dialogs &);
	///
	static void ComboInputCB(int, void *, Combox *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( FL_OBJECT *, long);
	/// Update the dialog.
	virtual void update();
	/// Apply from dialog
	virtual void apply();
	/// Cancel from dialog
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
	void BulletDepth(FL_OBJECT * ob);
	///
	void BulletPanel(FL_OBJECT * ob);
	///
	void BulletBMTable(FL_OBJECT * ob, long);
	///
	void checkReadOnly();
	///
	void CheckChoiceClass(FL_OBJECT * ob, long);
	///
	void UpdateLayoutDocument(BufferParams const & params);
	///
	void UpdateClassParams(BufferParams const & params);

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
	void paper_apply(BufferParams &);
	///
	bool class_apply(BufferParams &);
	///
	bool language_apply(BufferParams &);
	///
	bool options_apply(BufferParams &);
	///
	void bullets_apply(BufferParams &);

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

	/// Real GUI implementation.
	boost::scoped_ptr<FD_document> dialog_;
	///
	boost::scoped_ptr<FD_document_paper>    paper_;
	///
	boost::scoped_ptr<FD_document_class>    class_;
	///
	boost::scoped_ptr<FD_document_language> language_;
	///
	boost::scoped_ptr<FD_document_options>  options_;
	///
	boost::scoped_ptr<FD_document_bullet>   bullets_;
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
	///
	std::vector<string> lang_;
};


inline
xformsBC & FormDocument::bc()
{
	return bc_;
}

#endif
