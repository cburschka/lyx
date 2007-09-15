// -*- C++ -*-
/**
 * \file ControlDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck (modules)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLDOCUMENT_H
#define CONTROLDOCUMENT_H

#include "Dialog.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/types.h"
#include <boost/scoped_ptr.hpp>
#include <map>
#include <vector>

namespace lyx {

class BufferParams;
class TextClass;

namespace frontend {
	
///
typedef void const * BufferId;
///
typedef std::map<std::string, support::FileName> ModuleMap;

/** A controller for Document dialogs.
 */
class ControlDocument : public Controller {
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
	/// List of available modules
	std::vector<std::string> getModuleNames();
	/// Modules in use in current buffer
	std::vector<std::string> const & getSelectedModules();
	///
	std::string getModuleDescription(std::string modName) const;
	///
	std::vector<std::string> getPackageList(std::string modName) const;
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
	void loadModuleNames();
	///
	boost::scoped_ptr<BufferParams> bp_;
	/// List of names of available modules
	std::vector<std::string> moduleNames_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLDOCUMENT_H
