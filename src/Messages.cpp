/* \file Messages.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Messages.h"

#include "debug.h"

#include "support/docstring.h"
#include "support/environment.h"
#include "support/filetools.h"
#include "support/Package.h"
#include "support/unicode.h"

#include <boost/current_function.hpp>
#include <boost/regex.hpp>

#include <cerrno>

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

static boost::regex const reg("^([^\\[]*)\\[\\[[^\\]]*\\]\\]$");

using support::package;
using support::getEnv;
using support::setEnv;


// This version use the traditional gettext.
Messages::Messages(string const & l)
	: lang_(l), warned_(false)
{
	// strip off any encoding suffix, i.e., assume 8-bit po files
	string::size_type i = lang_.find(".");
	lang_ = lang_.substr(0, i);
	LYXERR(Debug::DEBUG) << BOOST_CURRENT_FUNCTION
		<< ": language(" << lang_ << ")" << endl;
}


void Messages::init()
{
	errno = 0;
	string const locale_dir = package().locale_dir().toFilesystemEncoding();
	char const * c = bindtextdomain(PACKAGE, locale_dir.c_str());
	int e = errno;
	if (e) {
		LYXERR(Debug::DEBUG)
			<< BOOST_CURRENT_FUNCTION << '\n'
			<< "Error code: " << errno << '\n'
			<< "Directory : " << package().locale_dir().absFilename() << '\n'
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

	string const oldLANGUAGE = getEnv("LANGUAGE");
	string const oldLC_ALL = getEnv("LC_ALL");
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
		// m is actually not a char const * but ucs4 data
		translated = reinterpret_cast<char_type const *>(msg);
	}

	if (!lang_.empty()) {
		// Reset everything as it was.
		setEnv("LANGUAGE", oldLANGUAGE);
		setEnv("LC_ALL", oldLC_ALL);
#ifdef HAVE_LC_MESSAGES
		setlocale(LC_MESSAGES, "");
#endif
	}

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

void Messages::init()
{
}


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
	std::locale loc_gl;
	///
	std::messages<char> const & mssg_gl;
	///
	catalog cat_gl;
};

} // namespace lyx

#endif
