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
#include "support/debug.h"
#include "support/filetools.h"
#include "support/Package.h"
#include "support/unicode.h"

#include <cerrno>
#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {


#ifdef ENABLE_NLS

#if 0

#include <locale>

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
		//lyxerr << "Messages: language(" << l
		//       << ") in dir(" << dir << ")" << endl;

		string const locale_dir = package().locale_dir().toFilesystemEncoding();
		cat_gl = mssg_gl.open(PACKAGE, loc_gl, locale_dir.c_str());

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
	locale loc_gl;
	///
	messages<char> const & mssg_gl;
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
#    include "../../intl/libintl.h"
#  endif

// This is a more traditional variant.
class Messages::Pimpl {
public:
	Pimpl(string const & l)
		: lang_(l)
	{
		//lyxerr << "Messages: language(" << l
		//       << ") in dir(" << dir << ")" << endl;

	}

	~Pimpl() {}

	docstring const get(string const & m) const
	{
		if (m.empty())
			return from_ascii(m);

		char * o = setlocale(LC_ALL, 0);
		string old;
		if (o)
			old = o;
		char * n = setlocale(LC_ALL, lang_.c_str());
		if (!n)
			// If we are unable to honour the request we just
			// return what we got in.
			return from_ascii(m);
		errno = 0;
		string const locale_dir = package().locale_dir().toFilesystemEncoding();
		char const * c = bindtextdomain(PACKAGE, locale_dir.c_str());
		int e = errno;
		if (e) {
			LYXERR(Debug::DEBUG, "Messages::get()" << '\n'
				<< "Error code: " << errno << '\n'
				<< "Lang, mess: " << lang_ << " " << m << '\n'
				<< "Directory : " << package().locale_dir().absFilename() << '\n'
				<< "Rtn value : " << c);
		}

		if (!bind_textdomain_codeset(PACKAGE, ucs4_codeset)) {
			LYXERR(Debug::DEBUG, "Messages::get()" << '\n'
				<< "Error code: " << errno << '\n'
				<< "Codeset   : " << ucs4_codeset << '\n');
		}

		textdomain(PACKAGE);

		char const * tmp = m.c_str();
		char const * msg = gettext(tmp);
		docstring translated;
		if (!msg) {
			lyxerr << "Undefined result from gettext" << endl;
			translated = from_ascii(tmp);
		} else if (msg == tmp) {
			//lyxerr << "Same as entered returned" << endl;
			translated = from_ascii(tmp);
		} else {
			LYXERR(Debug::DEBUG, "We got a translation");
			char_type const * ucs4 = reinterpret_cast<char_type const *>(msg);
			translated = ucs4;
		}
		setlocale(LC_ALL, old.c_str());
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

	docstring const get(string const & m) const
	{
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


docstring const Messages::get(string const & msg) const
{
	return pimpl_->get(msg);
}


} // namespace lyx
