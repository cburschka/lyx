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


#include "Qt2Base.h"
#include "Qt2BC.h"

#include <boost/scoped_ptr.hpp>

class ControlDocument;
class QDocumentDialog;
class LengthCombo;
class  QLineEdit;

class QDocument
	: public Qt2CB<ControlDocument, Qt2DB<QDocumentDialog> >
{
public:

	friend class QDocumentDialog;

	QDocument();

	void showPreamble();

private:
	/// Apply changes
	void apply();
	/// update
	void update_contents();
	/// build the dialog
	void build_dialog();
	/// save as default template
	void saveDocDefault();
	/// reset to default params
	void useClassDefaults();

	/// FIXME
	std::vector<string> lang_;
};

#endif // QDOCUMENT_H
