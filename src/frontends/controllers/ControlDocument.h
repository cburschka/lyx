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

#include "Dialog.h"
#include "support/types.h"
#include <boost/scoped_ptr.hpp>

class BufferParams;
class LyXTextClass;


/** A controller for Document dialogs.
 */
class ControlDocument : public Dialog::Controller {
public:
	///
	ControlDocument(Dialog &);
	///
	~ControlDocument();
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	LyXTextClass const & textClass() const;
	///
	BufferParams & params() const;
	///
	void setLanguage() const;
	///
	void saveAsDefault() const;
	///
	bool loadTextclass(lyx::textclass_type tc) const;
private:
	///
	boost::scoped_ptr<BufferParams> bp_;
};

#endif // CONTROLDOCUMENT_H
