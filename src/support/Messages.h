// -*- C++ -*-
/* \file Messages.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include "support/docstring.h"

#include <map>
#include <string>

namespace lyx {

///
class Messages {
public:
	/// messages in the language \p l.
	/// If \p l is empty, the language will be defined by the environment.
	Messages(std::string const & l = std::string());
	///
	docstring const get(std::string const & msg) const;
	///
	static void init();
	///
	static std::string const & defaultLanguage() { return main_lang_; }

private:
	///
	static void setDefaultLanguage();
	///
	static std::string main_lang_;
	///
	std::string lang_;
	/// Did we warn about unavailable locale already?
	mutable bool warned_;
	///
	typedef std::map<std::string, docstring> TranslationCache;
	/// Internal cache for gettext translated strings.
	/// This is needed for performance reason within \c updateLabels()
	/// under Windows.
	mutable TranslationCache cache_;
};

/// Access to the unique Messages object for the passed \p language.
/// Implementation is in LyX.cpp.
extern Messages & getMessages(std::string const & language);
/// Access to the unique Messages object used for GUI element.
/// Implementation is in LyX.cpp.
extern Messages & getGuiMessages();

} // namespace lyx

#endif
