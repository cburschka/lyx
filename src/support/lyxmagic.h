// -*- C++ -*-
/**
 * \file lyxmagic.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_MAGIC_H
#define LYX_MAGIC_H

#include "support/debug.h"

namespace lyx {

#ifdef HAVE_MAGIC_H

#include <magic.h>

class Magic {
public:
	Magic() : ok_(false) {
		cookie_ = magic_open(MAGIC_MIME);
		if (cookie_) {
			if (magic_load(cookie_, NULL) != 0)
				LYXERR(Debug::GRAPHICS, "Magic: couldn't load magic database - "
					<< magic_error(cookie_));
			else
				ok_ = true;
		}
	}

	~Magic() {
		if(cookie_)
			magic_close(cookie_);
	}

	// returns a string of the form "mime-type;encoding", or an empty string on error.
	std::string file(std::string const & name) const {
		if (!ok_)
			return std::string();

		char const * result = magic_file(cookie_, name.c_str());
		if (result)
			return result;
		else
			LYXERR(Debug::GRAPHICS, "Magic: couldn't query magic database - "
				   << magic_error(cookie_));

		return std::string();
	}

private:
	magic_t cookie_;
	bool ok_;
};

#else // !HAVE_MAGIC_T

// A dummy Magic class that always returns an empty result
class Magic {
public:
	Magic() {
		LYXERR(Debug::GRAPHICS, "Magic: libmagic support not configured");
	}

	std::string file(std::string const & ) const { return empty_string(); }
};

#endif // HAVE_MAGIC_T

}

#endif // LYX_MAGIC_H
