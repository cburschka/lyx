// -*- C++ -*-
/**
 * \file QDocument.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */


#ifndef QDOCUMENT_H
#define QDOCUMENT_H

#include <vector>
#include <boost/smart_ptr.hpp>

#include "Qt2Base.h"
#include "Qt2BC.h"
#include "QtLyXView.h"

#ifdef __GNUG_
#pragma interface
#endif

class ControlDocument;
class QDocumentDialog;
class BufferParams;
class LyXView;
class Dialogs;

class QDocument
	//FIXME !!!!! : public Qt2CB<ControlDocument, Qt2DB<QDocumentDialog> >
{
public:
	// FIXME!!!!!!!
	QDocument(LyXView *, Dialogs *);
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
	virtual Qt2BC & bc();
	/// Build the dialog
	virtual void build_dialog();
//	/// Filter the inputs
//	virtual bool input( FL_OBJECT *, long );
	/// Update the dialog.
	virtual void update_contents();
	/// Apply from dialog
	virtual void apply();
	/// Cancel from dialog
	virtual void cancel();

//	///
//	virtual QDialog* form() const;

#if 0
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
#endif
	/// Real GUI implementation.
	boost::scoped_ptr<QDocumentDialog> dialog_;
	///
	int ActCell;
	///
	int Confirmed;
	///
	int current_bullet_panel;
	///
	int current_bullet_depth;
//	///
//	FL_OBJECT * fbullet;
//	///
//	boost::scoped_ptr<Combox> combo_language;
//	///
//	boost::scoped_ptr<Combox> combo_doc_class;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, Qt2BC> bc_;
};


inline
Qt2BC & QDocument::bc()
{
	return bc_;
}

#endif // QDOCUMENT_H
