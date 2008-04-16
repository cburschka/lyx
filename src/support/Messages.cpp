/* \file Messages.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/Messages.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/environment.h"
#include "support/Package.h"
#include "support/unicode.h"

#include "support/assert.h"

#include <cerrno>

using namespace std;

namespace {

using lyx::docstring;
using lyx::from_ascii;

void cleanTranslation(docstring & trans) 
{
	/*
	  Some english words have different translations, depending on
	  context. In these cases the original string is augmented by
	  context information (e.g. "To:[[as in 'From page x to page
	  y']]" and "To:[[as in 'From format x to format y']]". This
	  means that we need to filter out everything in double square
	  brackets at the end of the string, otherwise the user sees
	  bogus messages. If we are unable to honour the request we
	  just return what we got in.
	*/
	size_t const pos1 = trans.find(from_ascii("[["));
	if (pos1 != docstring::npos) {
		size_t const pos2 = trans.find(from_ascii("]]"), pos1);
		if (pos2 != docstring::npos) 
			trans.erase(pos1, pos2 - pos1 + 2);
	}
}

}


#ifdef ENABLE_NLS

#  ifdef HAVE_LOCALE_H
#    include <locale.h>
#  endif

#  if HAVE_GETTEXT
#    include <libintl.h>      // use the header already in the system *EK*
#  else
#    include "../../intl/libintl.h"
#  endif

using namespace lyx::support;

namespace lyx {

// This version use the traditional gettext.
Messages::Messages(string const & l)
	: lang_(l), warned_(false)
{
	// strip off any encoding suffix, i.e., assume 8-bit po files
	size_t i = lang_.find(".");
	lang_ = lang_.substr(0, i);
	LYXERR(Debug::DEBUG, "language(" << lang_ << ")");
}


void Messages::init()
{
	errno = 0;
	string const locale_dir = package().locale_dir().toFilesystemEncoding();
	char const * c = bindtextdomain(PACKAGE, locale_dir.c_str());
	int e = errno;
	if (e) {
		LYXERR(Debug::DEBUG, "Error code: " << errno << '\n'
			<< "Directory : " << package().locale_dir().absFilename() << '\n'
			<< "Rtn value : " << c);
	}

	if (!bind_textdomain_codeset(PACKAGE, ucs4_codeset)) {
		LYXERR(Debug::DEBUG, "Error code: " << errno << '\n'
			<< "Codeset   : " << ucs4_codeset);
	}

	textdomain(PACKAGE);
}


docstring const Messages::get(string const & m) const
{
	if (m.empty())
		return docstring();

	// Look for the translated string in the cache.
	TranslationCache::iterator it = cache_.find(m);
	if (it != cache_.end())
		return it->second;

	// The string was not found, use gettext to generate it

	static string const oldLANGUAGE = getEnv("LANGUAGE");
	static string const oldLC_ALL = getEnv("LC_ALL");
	if (!lang_.empty()) {
		// This GNU extension overrides any language locale
		// wrt gettext.
		setEnv("LANGUAGE", lang_);
		// However, setting LANGUAGE does nothing when the
		// locale is "C". Therefore we set the locale to
		// something that is believed to exist on most
		// systems. The idea is that one should be able to
		// load German documents even without having de_DE
		// installed.
		setEnv("LC_ALL", "en_US");
#ifdef HAVE_LC_MESSAGES
		setlocale(LC_MESSAGES, "");
#endif
	}

	char const * m_c = m.c_str();
	char const * trans_c = gettext(m_c);
	docstring trans;
	if (!trans_c)
		LYXERR0("Undefined result from gettext");
	else if (trans_c == m_c) {
		LYXERR(Debug::DEBUG, "Same as entered returned");
		trans = from_ascii(m);
	} else {
		LYXERR(Debug::DEBUG, "We got a translation");
		// m is actually not a char const * but ucs4 data
		trans = reinterpret_cast<char_type const *>(trans_c);
	}

	cleanTranslation(trans);

	// Reset environment variables as they were.
	if (!lang_.empty()) {
		// Reset everything as it was.
		setEnv("LANGUAGE", oldLANGUAGE);
		setEnv("LC_ALL", oldLC_ALL);
#ifdef HAVE_LC_MESSAGES
		setlocale(LC_MESSAGES, "");
#endif
	}

	pair<TranslationCache::iterator, bool> result =
		cache_.insert(make_pair(m, trans));

	LASSERT(result.second, /**/);

	return result.first->second;
}

} // namespace lyx

#else // ENABLE_NLS
// This is the dummy variant.

namespace lyx {

Messages::Messages(string const & l) {}

void Messages::init()
{
}


docstring const Messages::get(string const & m) const
{
	docstring trans = from_ascii(m);
	cleanTranslation(trans);
	return trans;
}

} // namespace lyx

#endif

#if 0

-#include <locale>

namespace lyx {

// This version of the Pimpl utilizes the message capability of
// libstdc++ that is distributed with GNU G++.
class Messages::Pimpl {
public:
	typedef messages<char>::catalog catalog;

	Pimpl(string const & l)
		: lang_(l),
		  loc_gl(lang_.c_str()),
		  mssg_gl(use_facet<messages<char> >(loc_gl))
	{
		//LYXERR("Messages: language(" << l << ") in dir(" << dir << ")");

		string const locale_dir = package().locale_dir().toFilesystemEncoding();
		cat_gl = mssg_gl.open(PACKAGE, loc_gl, locale_dir.c_str());

	}

	~Pimpl()
	{
		mssg_gl.close(cat_gl);
	}

	docstring const get(string const & msg) const
	{
		return mssg_gl.get(cat_gl, 0, 0, msg);
	}
private:
	///
	string lang_;
	///
	locale loc_gl;
	///
	messages<char> const & mssg_gl;
	///
	catalog cat_gl;
};

} // namespace lyx

#endif
