// -*- C++ -*-
/**
 * \file QIndex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QINDEX_H
#define QINDEX_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetindex.h"

class Dialogs;
class LyXView;
class QIndexDialog;

class QIndex : public DialogBase {
public: 
	QIndex(LyXView *, Dialogs *);
	~QIndex();

	/// Apply changes
	void apply();
	/// close the connections
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update();

	/// create an Index inset
	void createIndex(string const &);
	/// edit an Index  inset
	void showIndex(InsetCommand * const);
 
	/// Real GUI implementation.
	QIndexDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// pointer to the inset if any
	InsetCommand * inset_;
	/// insets params
	InsetCommandParams params;
	/// is the inset we are reading from a readonly buffer ?
	bool readonly;
	
	/// Hide connection.
	SigC::Connection h_;
	/// Update connection.
	SigC::Connection u_;
	/// Inset hide connection.
	SigC::Connection ih_;
};

#endif // QINDEX_H
