// -*- C++ -*-
/**
 * \file FormDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORM_DOCUMENT_H
#define FORM_DOCUMENT_H

#include "FormDialogView.h"
#include "BranchList.h"

#include <boost/scoped_ptr.hpp>

#include "lyx_forms.h"
#include <vector>

class BufferParams;

namespace lyx {
namespace frontend {

class ControlDocument;
class FormColorpicker;
struct FD_document;
struct FD_document_paper;
struct FD_document_class;
struct FD_document_language;
struct FD_document_options;
struct FD_document_bullet;
struct FD_document_branch;

/** This class provides an XForms implementation of the FormDocument dialog.
 *  The table-layout-form here changes values for latex-tabulars
 */
class FormDocument
	: public FormController<ControlDocument, FormView<FD_document> > {
public:
	FormDocument(Dialog &);
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
	void branch_input(FL_OBJECT *);
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
	void branch_update(BufferParams const &);

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
	void branch_apply(BufferParams &);

	void rebuild_all_branches_browser();
	void rebuild_selected_branches_browser();

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
	boost::scoped_ptr<FD_document_branch>   branch_;
	///
	boost::scoped_ptr<FormColorpicker> picker_;
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
	std::vector<std::string> lang_;
	/// Contains all legal branches for this doc
	BranchList branchlist_;
};

} // namespace frontend
} // namespace lyx

#endif
