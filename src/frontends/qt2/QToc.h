// -*- C++ -*-
/**
 * \file QToc.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se> 
 */

#ifndef QTOC_H
#define QTOC_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"
#include "buffer.h"

class Dialogs;
class QTocDialog;

class QToc : public DialogBase {
public:
	QToc(LyXView *, Dialogs *);
	~QToc();

	// Build the dialog
	virtual void build();
	/// Update the dialog before showing it.
	virtual void update();
	/// close the connections
	virtual void close();

private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// create a Toc inset
	void createTOC(string const &);
	/// view a Toc inset
	void showTOC(InsetCommand * const);
	
	/// update the Toc
	void updateToc(int);

	/// Real GUI implementation.
	QTocDialog * dialog_;

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
	
	/// Hide connection.
	SigC::Connection h_;
	/// Update connection.
	SigC::Connection u_;
	/// Inset hide connection.
	SigC::Connection ih_;

	/// the toc list
	//std::vector <Buffer::TocItem> toclist;

	/// depth of list shown
	int depth;
};

#endif // QTOC_H
