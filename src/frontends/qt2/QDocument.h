// -*- C++ -*-
/**
 * \file QDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */


#ifndef QDOCUMENT_H
#define QDOCUMENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"
#include "Qt2BC.h"
//#include "QtLyXView.h"

#include <boost/scoped_ptr.hpp>

class ControlDocument;
class QDocumentDialog;

class QDocument
	: public Qt2CB<ControlDocument, Qt2DB<QDocumentDialog> >
{
public:
	///
	QDocument();
	///
	friend class QDocumentDialog;
private:
	/// Apply changes
	void apply();
	/// update
	void update_contents();
	/// build the dialog
	void build_dialog();
	///
	std::vector<string> lang_;
};


#endif // QDOCUMENT_H
