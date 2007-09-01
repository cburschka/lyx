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

namespace lyx {

class BufferParams;
class TextClass;

namespace frontend {

///
typedef void const * BufferId;

/** A controller for Document dialogs.
 */
class ControlDocument : public Dialog::Controller {
public:
	/// font family names for BufferParams::fontsDefaultFamily
	static char const * const fontfamilies[5];
	/// GUI names corresponding fontfamilies
	static char const * fontfamilies_gui[5];
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
	/// always true since we don't manipulate document contents
	virtual bool canApply() const { return true; }
	///
	TextClass const & textClass() const;
	///
	BufferParams & params() const;
	///
	BufferId id() const;
	///
	void setLanguage() const;
	///
	void saveAsDefault() const;
	///
	bool const isFontAvailable(std::string const & font) const;
	/// does this font provide Old Style figures?
	bool const providesOSF(std::string const & font) const;
	/// does this font provide true Small Caps?
	bool const providesSC(std::string const & font) const;
	/// does this font provide size adjustment?
	bool const providesScale(std::string const & font) const;
private:
	///
	boost::scoped_ptr<BufferParams> bp_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLDOCUMENT_H
