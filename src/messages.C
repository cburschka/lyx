/* \file messages.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "messages.h"
#include "support/filetools.h"
#include "support/environment.h"
#include "support/package.h"
#include "support/docstring.h"
#include "support/types.h"

#include <boost/current_function.hpp>
#include <boost/regex.hpp>

#include <cerrno>
#include <map>

using std::endl;
using std::string;

namespace lyx {

using support::package;
using support::getEnv;
using support::setEnv;


static boost::regex const reg("^([^\\[]*)\\[\\[[^\\]]*\\]\\]$");


#ifdef ENABLE_NLS


#if 0

-#include <locale>

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
#else

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

#  if HAVE_GETTEXT
#    include <libintl.h>      // use the header already in the system *EK*
#  else
#    include "../intl/libintl.h"
#  endif

// This is a more traditional variant.
class Messages::Pimpl {
public:
	Pimpl(string const & l)
		: lang_(l)
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
		lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION
				     << ": language(" << lang_ << ")" << endl;
	}

	~Pimpl() {}

	docstring const & get(string const & m) const
	{
		if (m.empty()) {
			dummy_string_.clear();
			return dummy_string_;
		}

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
		static bool warned = false;
		if (!warned && !lc_msgs) {
			warned = true;
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
			lyxerr[Debug::DEBUG]
				<< BOOST_CURRENT_FUNCTION << '\n'
				<< "Error code: " << errno << '\n'
				<< "Lang, mess: " << lang_ << " " << m << '\n'
				<< "Directory : " << package().locale_dir() << '\n'
				<< "Rtn value : " << c << endl;
		}
#ifdef WORDS_BIGENDIAN
		static const char * codeset = "UCS-4BE";
#else
		static const char * codeset = "UCS-4LE";
#endif
		if (!bind_textdomain_codeset(PACKAGE, codeset)) {
			lyxerr[Debug::DEBUG]
				<< BOOST_CURRENT_FUNCTION << '\n'
				<< "Error code: " << errno << '\n'
				<< "Codeset   : " << codeset << '\n'
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
			lyxerr[Debug::DEBUG] << "We got a translation" << endl;
			char_type const * ucs4 = reinterpret_cast<char_type const *>(msg);
			translated = ucs4;
		}
#ifdef HAVE_LC_MESSAGES
		setlocale(LC_MESSAGES, lang.c_str());
#endif
		setlocale(LC_CTYPE, oldCTYPE.c_str());

		std::pair<TranslationCache::iterator, bool> result = 
			cache_.insert(std::make_pair(m, translated));

		if (!result.second) {
			lyxerr << "WARNING: cannot fill-in gettext cache in Messages::get()!" << endl;
			dummy_string_ = translated;
			return dummy_string_;
		}

		return result.first->second;
	}
private:
	///
	string lang_;
	typedef std::map<string, docstring> TranslationCache;
	/// Internal cache for gettext translated strings.
	/// This is needed for performance reason within \c updateLabels()
	/// under Windows.
	mutable TranslationCache cache_;
	/// Dummy string which serves as a storage place if something goes
	/// wrong with the translation cache.
	mutable docstring dummy_string_;
};
#endif

#else // ENABLE_NLS
// This is the dummy variant.
class Messages::Pimpl {
public:
	Pimpl(string const &) {}

	~Pimpl() {}

	docstring const get(string const & m) const
	{
		// See comment above
		boost::smatch sub;
		if (regex_match(m, sub, reg))
			return from_ascii(sub.str(1));
		else
			return from_ascii(m);
	}
};
#endif


Messages::Messages()
	: pimpl_(new Pimpl(""))
{}


Messages::Messages(string const & l)
	: pimpl_(new Pimpl(l))
{}


// We need this for the sake of scoped_ptr
Messages::~Messages()
{}


docstring const & Messages::get(string const & msg) const
{
	return pimpl_->get(msg);
}


} // namespace lyx
