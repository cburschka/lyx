// -*- C++ -*-
/**
 * \file ControlDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLDOCUMENT_H
#define CONTROLDOCUMENT_H

#include "ControlDialog_impl.h"
#include "bufferparams.h"

#include <boost/scoped_ptr.hpp>

class Language;


/** A controller for Document dialogs.
 */
class ControlDocument : public ControlDialogBD {
public:
	///
	ControlDocument(LyXView &, Dialogs &);
	///
	~ControlDocument();
	///
	void setLanguage();
	///
	LyXTextClass textClass();
	///
	BufferParams & params();
	///
	void saveAsDefault();
	///
	void classApply();
	///
	bool loadTextclass(lyx::textclass_type tc) const;
private:
	/// apply settings
	void apply();
	/// set the params before show or update
	void setParams();
	///
	boost::scoped_ptr<BufferParams> bp_;
};

#endif // CONTROLDOCUMENT_H
