/**
 * \file texstream.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "texstream.h"

#include "TexRow.h"

#include "support/lstrings.h"
#include "support/unicode.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iconv.h>
#include <locale>

using namespace std;

using lyx::support::contains;
using lyx::support::split;


namespace lyx {


otexrowstream::otexrowstream(odocstream & os)
	: os_(os), texrow_(make_unique<TexRow>())
{}


otexrowstream::~otexrowstream() = default;


unique_ptr<TexRow> otexrowstream::releaseTexRow()
{
	auto p = make_unique<TexRow>();
	swap(texrow_, p);
	return p;
}


void otexrowstream::append(docstring const & str, TexRow texrow)
{
	os_ << str;
	texrow_->append(move(texrow));
}


void otexrowstream::put(char_type const & c)
{
	os_.put(c);
	if (c == '\n')
		texrow_->newline();
}


void otexstream::put(char_type const & c)
{
	if (protectspace_) {
		if (!canbreakline_ && c == ' ')
			os() << "{}";
		protectspace_ = false;
	}
	otexrowstream::put(c);
	lastChar(c);
}


BreakLine breakln;
SafeBreakLine safebreakln;


otexstream & operator<<(otexstream & ots, BreakLine)
{
	if (ots.canBreakLine()) {
		ots.otexrowstream::put('\n');
		ots.lastChar('\n');
	}
	ots.protectSpace(false);
	return ots;
}


otexstream & operator<<(otexstream & ots, SafeBreakLine)
{
	otexrowstream & otrs = ots;
	if (ots.canBreakLine()) {
		otrs << "%\n";
		ots.lastChar('\n');
	}
	ots.protectSpace(false);
	return ots;
}


otexrowstream & operator<<(otexrowstream & ots, odocstream_manip pf)
{
	ots.os() << pf;
	if (pf == static_cast<odocstream_manip>(endl)) {
		ots.texrow().newline();
	}
	return ots;
}

otexstream & operator<<(otexstream & ots, odocstream_manip pf)
{
	otexrowstream & otrs = ots;
	otrs << pf;
	if (pf == static_cast<odocstream_manip>(endl)) {
		ots.lastChar('\n');
	}
	return ots;
}


otexrowstream & operator<<(otexrowstream & ots, docstring const & s)
{
	ots.os() << s;
	ots.texrow().newlines(count(s.begin(), s.end(), '\n'));
	return ots;
}


otexstream & operator<<(otexstream & ots, docstring const & s)
{
	size_t const len = s.length();

	// Check whether there's something to output
	if (len == 0)
		return ots;
	otexrowstream & otrs = ots;
	if (ots.protectSpace()) {
		if (!ots.canBreakLine() && s[0] == ' ')
			otrs << "{}";
		ots.protectSpace(false);
	}

	if (contains(s, 0xF0000)) {
		// Some encoding changes for the underlying stream are embedded
		// in the docstring. The encoding names to be used are enclosed
		// between the code points 0xF0000 and 0xF0001, the first two
		// characters of plane 15, which is a Private Use Area whose
		// codepoints don't have any associated glyph.
		docstring s1;
		docstring s2 = split(s, s1, 0xF0000);
		while (true) {
			if (!s1.empty())
				otrs << s1;
			if (s2.empty())
				break;
			docstring enc;
			docstring const s3 = split(s2, enc, 0xF0001);
			if (!contains(s2, 0xF0001))
				s2 = split(enc, s1, 0xF0000);
			else {
				otrs << setEncoding(to_ascii(enc));
				s2 = split(s3, s1, 0xF0000);
			}
		}
	} else
		otrs << s;

	if (len > 1)
		ots.canBreakLine(s[len - 2] != '\n');
	ots.lastChar(s[len - 1]);
	return ots;
}


otexrowstream & operator<<(otexrowstream & ots, string const & s)
{
	ots << from_utf8(s);
	return ots;
}


otexstream & operator<<(otexstream & ots, string const & s)
{
	ots << from_utf8(s);
	return ots;
}


otexrowstream & operator<<(otexrowstream & ots, char const * s)
{
	ots << from_utf8(s);
	return ots;
}


otexstream & operator<<(otexstream & ots, char const * s)
{
	ots << from_utf8(s);
	return ots;
}


otexrowstream & operator<<(otexrowstream & ots, char c)
{
	ots.put(c);
	return ots;
}


otexstream & operator<<(otexstream & ots, char c)
{
	ots.put(c);
	return ots;
}


template <typename Type>
otexrowstream & operator<<(otexrowstream & ots, Type value)
{
	ots.os() << value;
	return ots;
}

template otexrowstream & operator<< <SetEnc>(otexrowstream & os, SetEnc);
template otexrowstream & operator<< <double>(otexrowstream &, double);
template otexrowstream & operator<< <int>(otexrowstream &, int);
template otexrowstream & operator<< <unsigned int>(otexrowstream &,
												   unsigned int);
template otexrowstream & operator<< <unsigned long>(otexrowstream &,
													unsigned long);

#ifdef LYX_USE_LONG_LONG
template otexrowstream & operator<< <unsigned long long>(otexrowstream &,
                                                         unsigned long long);
#endif


template <typename Type>
otexstream & operator<<(otexstream & ots, Type value)
{
	ots.os() << value;
	ots.lastChar(0);
	ots.protectSpace(false);
	return ots;
}

template otexstream & operator<< <SetEnc>(otexstream & os, SetEnc);
template otexstream & operator<< <double>(otexstream &, double);
template otexstream & operator<< <int>(otexstream &, int);
template otexstream & operator<< <unsigned int>(otexstream &, unsigned int);
template otexstream & operator<< <unsigned long>(otexstream &, unsigned long);
#ifdef LYX_USE_LONG_LONG
template otexstream & operator<< <unsigned long long>(otexstream &, unsigned long long);
#endif

}
