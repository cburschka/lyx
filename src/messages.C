* \file messages.C
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

using std::endl;


Messages::Messages(string const & l, string const & dir)
	: lang_(l), localedir_(dir),
	  loc_gl(lang_.c_str()),
	  mssg_gl(std::use_facet<std::messages<char> >(loc_gl))
{
	lyxerr << "Messages: language(" << l << ") in dir(" << dir << ")" << endl;

	cat_gl = mssg_gl.open("lyx", loc_gl, localedir_.c_str());
}


Messages::~Messages()
{
	mssg_gl.close(cat_gl);
}


string const Messages::get(string const & msg) const
{
	return mssg_gl.get(cat_gl, 0, 0, msg);
}
