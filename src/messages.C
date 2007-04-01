/* \file messages.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "messages.h"

#include "debug.h"

#include "support/docstring.h"
#include "support/environment.h"
#include "support/filetools.h"
#include "support/package.h"
#include "support/unicode.h"

#include <boost/current_function.hpp>
#include <boost/regex.hpp>

#include <cerrno>

namespace {
boost::regex const reg("^([^\\[]*)\\[\\[[^\\]]*\\]\\]$");
};

#ifdef ENABLE_NLS

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

#  if HAVE_GETTEXT
#    include <libintl.h>      // use the header already in the system *EK*
#  else
#    include "../intl/libintl.h"
#  endif

using std::endl;
using std::make_pair;
using std::map;
using std::string;

namespace lyx {

using support::package;
using support::getEnv;
using support::setEnv;


// This version use the traditional gettext.
Messages::Messages(string const & l)
	: lang_(l), warned_(false)
{
	if ( lang_.empty() ) {
		char const * lc_msgs = 0;
#ifdef HAVE_LC_MESSAGES
		lc_msgs = setlocale(LC_MESSAGES, NULL);
#endif
		lang_ = lc_msgs ? lc_msgs : "";
	}
	// strip off any encoding suffix, i.e., assume 8-bit po files
	string::size_type i = lang_.find(".");
	lang_ = lang_.substr(0, i);
	LYXERR(Debug::DEBUG) << BOOST_CURRENT_FUNCTION
		<< ": language(" << lang_ << ")" << endl;
}


docstring const Messages::get(string const & m) const
{
	if (m.empty())
		return docstring();

	// Look for the translated string in the cache.
	TranslationCache::iterator it = cache_.find(m);
	if (it != cache_.end())
		return it->second;
	// The string was not found, use gettext to generate it:

	// In this order, see support/filetools.C:
	string lang = getEnv("LC_ALL");
	if (lang.empty()) {
		lang = getEnv("LC_MESSAGES");
		if (lang.empty()) {
			lang = getEnv("LANG");
			if (lang.empty())
				lang = "C";
		}
	}
#ifdef HAVE_LC_MESSAGES
	char const * lc_msgs = setlocale(LC_MESSAGES, lang_.c_str());
#endif
	// setlocale fails (returns NULL) if the corresponding locale
	// is not installed.
	// On windows (mingw and cygwin) it always returns NULL.
	// Since this method gets called for every translatable
	// buffer string like e.g. "Figure:" we warn only once.
#if !defined(_WIN32) && !defined(__CYGWIN__)
	if (!warned_ && !lc_msgs) {
		warned_ = true;
		lyxerr << "Locale " << lang_ << " could not be set" << endl;
	}
#endif
	// CTYPE controls what getmessage thinks what encoding the po file uses
	char const * lc_ctype = setlocale(LC_CTYPE, NULL);
	string oldCTYPE = lc_ctype ? lc_ctype : "";

	setlocale(LC_CTYPE, lang_.c_str());
	errno = 0;
	char const * c = bindtextdomain(PACKAGE, package().locale_dir().c_str());
	int e = errno;
	if (e) {
		LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION << '\n'
			<< "Error code: " << errno << '\n'
			<< "Lang, mess: " << lang_ << " " << m << '\n'
			<< "Directory : " << package().locale_dir() << '\n'
			<< "Rtn value : " << c << endl;
	}

	if (!bind_textdomain_codeset(PACKAGE, ucs4_codeset)) {
		LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION << '\n'
			<< "Error code: " << errno << '\n'
			<< "Codeset   : " << ucs4_codeset << '\n'
			<< endl;
	}

	textdomain(PACKAGE);
	char const * tmp = m.c_str();
	char const * msg = gettext(tmp);
	docstring translated;
	if (!msg || msg == tmp) {
		if (!msg)
			lyxerr << "Undefined result from gettext" << endl;
		//else
		//	lyxerr << "Same as entered returned" << endl;
		// Some english words have different translations,
		// depending on context. In these cases the original
		// string is augmented by context information (e.g.
		// "To:[[as in 'From page x to page y']]" and
		// "To:[[as in 'From format x to format y']]".
		// This means that we need to filter out everything
		// in double square brackets at the end of the
		// string, otherwise the user sees bogus messages.
		// If we are unable to honour the request we just
		// return what we got in.
		boost::smatch sub;
		if (regex_match(m, sub, reg))
			translated = from_ascii(sub.str(1));
		else
			translated = from_ascii(tmp);
	} else {
		LYXERR(Debug::DEBUG) << "We got a translation" << endl;
		char_type const * ucs4 = reinterpret_cast<char_type const *>(msg);
		translated = ucs4;
	}
#ifdef HAVE_LC_MESSAGES
	setlocale(LC_MESSAGES, lang.c_str());
#endif
	setlocale(LC_CTYPE, oldCTYPE.c_str());

	std::pair<TranslationCache::iterator, bool> result = 
		cache_.insert(std::make_pair(m, translated));

	BOOST_ASSERT(result.second);

	return result.first->second;
}

} // namespace lyx

#else // ENABLE_NLS
// This is the dummy variant.

using std::endl;
using std::make_pair;
using std::map;
using std::string;

namespace lyx {

Messages::Messages(string const & l) {}


docstring const Messages::get(string const & m) const
{
	// See comment above
	boost::smatch sub;
	if (regex_match(m, sub, reg))
		return from_ascii(sub.str(1));
	else
		return from_ascii(m);
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
	typedef std::messages<char>::catalog catalog;

	Pimpl(string const & l)
		: lang_(l),
		  loc_gl(lang_.c_str()),
		  mssg_gl(std::use_facet<std::messages<char> >(loc_gl))
	{
		//lyxerr << "Messages: language(" << l
		//       << ") in dir(" << dir << ")" << endl;

		cat_gl = mssg_gl.open(PACKAGE, loc_gl, package().locale_dir().c_str());

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
	std::locale loc_gl;
	///
	std::messages<char> const & mssg_gl;
	///
	catalog cat_gl;
};

} // namespace lyx

#endif
