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

extern BufferList bufferlist;

class text_fits {
public:
	text_fits(Buffer * b, unsigned short p)
		: buf(b), pw(p) {}
	bool operator()(TextCache::value_type & vt) {
		if (vt->buffer() == buf && vt->paperWidth() == pw) return true;
		return false;
	}
private:
	Buffer * buf;
	unsigned short pw;
};


LyXText * TextCache::findFit(Buffer * b, unsigned short p)
{
	Cache::iterator it = find_if(cache.begin(), cache.end(),
				     text_fits(b, p));
	if (it != cache.end()) {
		LyXText * tmp = (*it);
		cache.erase(it);
		return tmp;
	}
	return 0;
}


class show_text {
public:
	show_text(ostream & o) : os(o) {}
	void operator()(TextCache::value_type & vt) {
		os << "\tBuffer: " << vt->buffer()
		   << "\tWidth: " << vt->paperWidth() << endl;
	}
private:
	ostream & os;
};

void TextCache::show(ostream & os, string const & str)
{
	os << "TextCache: " << str << endl;
	for_each(cache.begin(), cache.end(), show_text(os));
}


void TextCache::show(ostream & os, LyXText * lt)
{
	show_text st(os);
	st(lt);
}


void TextCache::add(LyXText * text)
{
	lyxerr.debug() << "TextCache::add " << text;
	if (bufferlist.isLoaded(text->buffer())) {
		cache.push_back(text);
		lyxerr.debug() << " added" << endl;
	} else {
		delete text;
		lyxerr.debug() << " deleted" << endl;
	}
}


class delete_text {
public:
	void operator()(TextCache::value_type & vt) {
		delete vt;
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
		if (vt->buffer() == buf) return true;
		return false;
	}
private:
	Buffer * buf;
};


void TextCache::removeAllWithBuffer(Buffer * buf)
{
	Cache::iterator it = remove_if(cache.begin(), cache.end(),
				       has_buffer(buf));
	if (it != cache.end()) {
		if (lyxerr.debugging()) {
			lyxerr.debug() << "TextCache::removeAllWithbuffer "
				"Removing:\n";
			for_each(it, cache.end(), show_text(lyxerr));
			lyxerr << endl;
		}
		for_each(it, cache.end(), delete_text());
		cache.erase(it, cache.end());
	}
}

// Global instance
TextCache textcache;
