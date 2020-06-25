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
	/// dummy instantiation: no translation is done
	Messages() {}
	/// messages in the language \p l.
	Messages(std::string const & l);
	/// Return the translation of message \c msg, or the original
	/// string if no context was found. Context is always removed.
	docstring const get(std::string const & msg) const;
	///
	docstring const getIfFound(std::string const & msg) const;
	/// What is the language associated with this translation?
	std::string language() const;
	/// Is an (at least partial) translation of language with code \p c available?
	static bool available(std::string const & c);
	///
	static void guiLanguage(std::string const & l) { gui_lang_ = l; }
	///
	static std::string const & guiLanguage() { return gui_lang_; }

private:
	/// Read the strings from the .mo file. Returns true on success.
	bool readMoFile();
	///
	std::string lang_;
	///
	typedef std::map<std::string, docstring> TranslationMap;
	TranslationMap trans_map_;
	/// The language used by the Gui
	static std::string gui_lang_;
};

/// Access to the unique Messages object for the passed \p language.
/// Implementation is in LyX.cpp.
extern Messages const & getMessages(std::string const & language);
/// Access to the unique Messages object used for GUI element.
/// Implementation is in LyX.cpp.
extern Messages const & getGuiMessages();

/// Remove the context suffix from \p trans
extern void cleanTranslation(docstring & trans);

} // namespace lyx

#endif
