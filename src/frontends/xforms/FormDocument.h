// -*- C++ -*-
/**
 * \file FormDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

#include <boost/scoped_ptr.hpp>

#include <vector>

class ControlDocument;

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
class FormDocument : public FormCB<ControlDocument, FormDB<FD_document> > {
public:
	FormDocument();
	///
	static void ComboInputCB(int, void *, Combox *);
private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual ButtonPolicy::SMInput input( FL_OBJECT *, long);
	/// Update the dialog.
	virtual void update();
	/// Apply from dialog
	virtual void apply();

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
	void CheckChoiceClass();
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

	/// Real GUI implementation.
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
	///
	std::vector<string> lang_;
};

#endif
