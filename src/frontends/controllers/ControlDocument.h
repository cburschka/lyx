// -*- C++ -*-
/**
 * \file ControlDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLDOCUMENT_H
#define CONTROLDOCUMENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include <boost/scoped_ptr.hpp>
#include "ControlDialog_impl.h"
#include "LString.h"
#include "bufferparams.h"

#include <vector>

class Language;


/** A controller for Document dialogs.
 */
class ControlDocument : public ControlDialogBD {
public:
	///
	ControlDocument(LyXView &, Dialogs &);
	~ControlDocument();
	/// show latex preamble
	void showPreamble();
	///
	void setLanguage();
	///
	LyXTextClass textClass();
	///
	BufferParams & params();
	///
	void saveAsDefault();
        ///
        bool classApply();
   
private:
	/// apply settings
	void apply();
	/// set the params before show or update
	void setParams();
	///
	boost::scoped_ptr<BufferParams> bp_;
};

#endif // CONTROLDOCUMENT_H


