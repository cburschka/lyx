/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team
 *
 *           This file is Copyright 2000
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include "TextCache.h"
#include "buffer.h"
#include "bufferlist.h"

using std::ostream;
using std::for_each;
using std::remove_if;
using std::find_if;
using std::endl;
using std::make_pair;

extern BufferList bufferlist;

class text_fits {
public:
	text_fits(Buffer * b, int p)
		: buf(b), pw(p) {}
	bool operator()(TextCache::value_type & vt) {
		if (vt.first == buf && vt.second.first == pw)
			return true;
		return false;
	}
private:
	Buffer * buf;
	int pw;
};


LyXText * TextCache::findFit(Buffer * b, int p)
{
	Cache::iterator it = find_if(cache.begin(), cache.end(),
				     text_fits(b, p));
	if (it != cache.end()) {
		LyXText * tmp = (*it).second.second;
		cache.erase(it);
		return tmp;
	}
	return 0;
}


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


void TextCache::add(Buffer *buf, int workwidth, LyXText * text)
{
	lyxerr.debug() << "TextCache::add " << text;
	if (bufferlist.isLoaded(buf)) {
		cache[buf] = make_pair(workwidth, text);
		lyxerr.debug() << " added" << endl;
	} else {
		delete text;
		lyxerr.debug() << " deleted" << endl;
	}
}


class delete_text {
public:
	void operator()(TextCache::value_type & vt) {
		delete vt.second.second;
	}
};


void TextCache::clear()
{
	for_each(cache.begin(), cache.end(), delete_text());
	cache.clear();
}


class has_buffer {
public:
	has_buffer(Buffer * b)
		: buf(b) {}
	bool operator()(TextCache::value_type & vt) {
		if (vt.first == buf) return true;
		return false;
	}
private:
	Buffer const * buf;
};


void TextCache::removeAllWithBuffer(Buffer * buf)
{
	cache.erase(buf);
}

// Global instance
TextCache textcache;
