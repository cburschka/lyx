/* \file messages.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "messages.h"
#include "debug.h"


#ifdef ENABLE_NLS

#if 0

#include <locale>

// This version of the Pimpl utilizes the message capability of
// libstdc++ that is distributed with GNU G++
class Messages::Pimpl {
public:
	typedef std::messages<char>::catalog catalog;

	Pimpl(string const & l, string const & dir)
		: lang_(l), localedir_(dir),
		  loc_gl(lang_.c_str()),
		  mssg_gl(std::use_facet<std::messages<char> >(loc_gl))
	{
		//lyxerr << "Messages: language(" << l
		//       << ") in dir(" << dir << ")" << std::endl;

		cat_gl = mssg_gl.open(PACKAGE, loc_gl, localedir_.c_str());

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
	string localedir_;
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
	Pimpl(string const & l, string const & dir)
		: lang_(l), localedir_(dir)
	{
		//lyxerr << "Messages: language(" << l
		//       << ") in dir(" << dir << ")" << std::endl;

	      bindtextdomain(PACKAGE, localedir_.c_str());
	      textdomain(PACKAGE);
	}

	~Pimpl() {}

	string const get(string const & m) const
	{
		char * old = strdup(setlocale(LC_ALL, 0));
		setlocale(LC_ALL, lang_.c_str());
		const char* msg = gettext(m.c_str());
		setlocale(LC_ALL, old);
		free(old);
		return string(msg);
	}
private:
	///
	string lang_;
	///
	string localedir_;
};
#endif

#else // ENABLE_NLS
// This is the dummy variant.
class Messages::Pimpl {
public:
	Pimpl(string const &, string const &) {}

	~Pimpl() {}

	string const get(string const & m) const
	{
		return m;
	}
};
#endif

Messages::Messages(string const & l, string const & dir)
	: pimpl_(new Pimpl(l, dir))
{}


// We need this for the sake of scoped_ptr
Messages::~Messages()
{}


string const Messages::get(string const & msg) const
{
	return pimpl_->get(msg);
}
