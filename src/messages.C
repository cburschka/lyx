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

#include <boost/regex.hpp>

using lyx::support::package;
using lyx::support::getEnv;
using lyx::support::setEnv;

using std::string;


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
		//       << ") in dir(" << dir << ")" << std::endl;

		cat_gl = mssg_gl.open(PACKAGE, loc_gl, package().locale_dir().c_str());

	}

	~Pimpl()
	{
		mssg_gl.close(cat_gl);
	}

	string const get(string const & msg) const
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
		if ( lang_.empty() )
			lang_ = setlocale(LC_MESSAGES, NULL);
		// strip off any encoding suffix, i.e., assume 8-bit po files
		string::size_type i = lang_.find(".");
		lang_ = lang_.substr(0, i);
		lyxerr << "Messages: language(" << lang_ << ")" << std::endl;
	}

	~Pimpl() {}

	string const get(string const & m) const
	{
		if (m.empty())
			return m;

		//string oldMSG = setlocale(LC_MESSAGES, NULL);
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
		
		char const * works = setlocale(LC_MESSAGES, lang_.c_str());
		// CTYPE controls what getmessage thinks what encoding the po file uses
		string oldCTYPE = setlocale(LC_CTYPE, NULL);
		setlocale(LC_CTYPE, lang_.c_str());
		errno = 0;
		char const * c = bindtextdomain(PACKAGE, package().locale_dir().c_str());
		int e = errno;
		if (e) {
			lyxerr << "Error code: " << errno << std::endl;
			lyxerr << "Lang, mess: " << lang_ << " " << m << std::endl;
			lyxerr << "Directory:  " << package().locale_dir() << std::endl;
			lyxerr << "Rtn value:  " << c << std::endl;
		}
		textdomain(PACKAGE);
		const char* msg = gettext(m.c_str());
		string translated(works ? msg : m);
		// Some english words have different translations, depending
		// on context. In these cases the original string is
		// augmented by context information (e.g.
		// "To:[[as in 'From page x to page y']]" and
		// "To:[[as in 'From format x to format y']]".
		// This means that we need to filter out everything in
		// double square brackets at the end of the string,
		// otherwise the user sees bogus messages.
		// If we are unable to honour the request we just
		// return what we got in.
		static boost::regex const reg("^([^\\[]*)\\[\\[[^\\]]*\\]\\]$");
		boost::smatch sub;
		if (regex_match(translated, sub, reg))
			translated = sub.str(1);
		setlocale(LC_MESSAGES, lang.c_str());
		setlocale(LC_CTYPE, oldCTYPE.c_str());
		return translated;
	}
private:
	///
	string lang_;
};
#endif

#else // ENABLE_NLS
// This is the dummy variant.
class Messages::Pimpl {
public:
	Pimpl(string const &) {}

	~Pimpl() {}

	string const get(string const & m) const
	{
		return m;
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


string const Messages::get(string const & msg) const
{
	return pimpl_->get(msg);
}
