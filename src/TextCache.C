/**
 * \file TextCache.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TextCache.h"
#include "lyxtext.h"
#include "bufferlist.h"
#include "debug.h"

#include <algorithm>

using std::endl;
using std::find_if;
using std::for_each;
using std::make_pair;

using std::ostream;

extern BufferList bufferlist;

namespace {

class text_fits {
public:
	text_fits(Buffer * b, int p)
		: buf(b), pw(p) {}
	bool operator()(TextCache::value_type const & vt) const {
		if (vt.first == buf && vt.second.first == pw)
			return true;
		return false;
	}
private:
	Buffer * buf;
	int pw;
};


class show_text {
public:
	show_text(ostream & o) : os(o) {}
	void operator()(TextCache::value_type const & vt) {
		os << "\tBuffer: " << vt.first
		   << "\tWidth: " << vt.second.first << endl;
	}
private:
	ostream & os;
};


class delete_text {
public:
	void operator()(TextCache::value_type & vt) {
		delete vt.second.second;
	}
};

} // namespace anon


LyXText * TextCache::findFit(Buffer * b, int p)
{
	Cache::iterator it = find_if(cache.begin(), cache.end(),
				     text_fits(b, p));
	if (it != cache.end()) {
		LyXText * tmp = it->second.second;
		cache.erase(it);
		return tmp;
	}
	return 0;
}


void TextCache::show(ostream & os, string const & str)
{
	os << "TextCache: " << str << endl;
	for_each(cache.begin(), cache.end(), show_text(os));
}


void TextCache::show(ostream & os, TextCache::value_type const & vt)
{
	show_text st(os);
	st(vt);
}


void TextCache::add(Buffer * buf, int workwidth, LyXText * text)
{
	lyxerr[Debug::INFO] << "TextCache::add " << text;
	if (bufferlist.isLoaded(buf)) {
		cache[buf] = make_pair(workwidth, text);
		lyxerr[Debug::INFO] << " added" << endl;
	} else {
		delete text;
		lyxerr[Debug::INFO] << " deleted" << endl;
	}
}


void TextCache::clear()
{
	for_each(cache.begin(), cache.end(), delete_text());
	cache.clear();
}


void TextCache::removeAllWithBuffer(Buffer * buf)
{
	cache.erase(buf);
}

// Global instance
TextCache textcache;
