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


void otexrowstream::put(char_type const & c)
{
	os_.put(c);
	if (c == '\n')
		texrow_->newline();
}


void otexstream::put(char_type const & c)
{
	bool isprotected = false;
	if (protectspace_) {
		if (!canbreakline_ && c == ' ') {
			os() << "{}";
			isprotected = true;
		}
		protectspace_ = false;
	}
	if (terminate_command_) {
		if ((c == ' ' || c == '\0' || c == '\n') && !isprotected)
			// A space or line break follows. Terminate with brackets.
			os() << "{}";
		else if (c != '\\' && c != '{' && c != '}')
			// Non-terminating character follows. Terminate with space.
			os() << " ";
		terminate_command_ = false;
	}
	otexrowstream::put(c);
	lastChar(c);
}


size_t otexstringstream::length()
{
	auto pos = ods_.tellp();
	return (pos >= 0) ? size_t(pos) : 0;
}


TexString otexstringstream::release()
{
	TexString ts(ods_.str(), move(texrow()));
	// reset this
	texrow() = TexRow();
	ods_.clear();
	ods_.str(docstring());
	return ts;
}


BreakLine breakln;
SafeBreakLine safebreakln;
TerminateCommand termcmd;


otexstream & operator<<(otexstream & ots, BreakLine)
{
	if (ots.canBreakLine()) {
		if (ots.terminateCommand())
			ots << "{}";
		ots.otexrowstream::put('\n');
		ots.lastChar('\n');
	}
	ots.protectSpace(false);
	ots.terminateCommand(false);
	return ots;
}


otexstream & operator<<(otexstream & ots, SafeBreakLine)
{
	otexrowstream & otrs = ots;
	if (ots.canBreakLine()) {
	    if (ots.terminateCommand())
			otrs << "{}";
		otrs << "%\n";
		ots.lastChar('\n');
	}
	ots.protectSpace(false);
	ots.terminateCommand(false);
	return ots;
}


otexstream & operator<<(otexstream & ots, TerminateCommand)
{
	ots.terminateCommand(true);
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


otexrowstream & operator<<(otexrowstream & ots, TexString ts)
{
	ts.validate();
	ots.os() << move(ts.str);
	ots.texrow().append(move(ts.texrow));
	return ots;
}


otexstream & operator<<(otexstream & ots, TexString ts)
{
	size_t const len = ts.str.length();
	// Check whether there is something to output
	if (len == 0)
		return ots;

	otexrowstream & otrs = ots;
	bool isprotected = false;
	char_type const c = ts.str[0];
	if (ots.protectSpace()) {
		if (!ots.canBreakLine() && c == ' ') {
			otrs << "{}";
			isprotected = true;
		}
		ots.protectSpace(false);
	}
	if (ots.terminateCommand()) {
		if ((c == ' ' || c == '\0' || c == '\n') && !isprotected)
			// A space or line break follows. Terminate with brackets.
			otrs << "{}";
		else if (c != '\\' && c != '{' && c != '}')
			// Non-terminating character follows. Terminate with space.
			otrs << " ";
		ots.terminateCommand(false);
	}

	if (len > 1)
		ots.canBreakLine(ts.str[len - 2] != '\n');
	ots.lastChar(ts.str[len - 1]);

	otrs << move(ts);
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
	bool isprotected = false;
	char_type const c = s[0];
	if (ots.protectSpace()) {
		if (!ots.canBreakLine() && c == ' ') {
			otrs << "{}";
			isprotected = true;
		}
		ots.protectSpace(false);
	}
	if (ots.terminateCommand()) {
		if ((c == ' ' || c == '\0' || c == '\n') && !isprotected)
			// A space or line break follows. Terminate with brackets.
			otrs << "{}";
		else if (c != '\\' && c != '{' && c != '}')
			// Non-terminating character follows. Terminate with space.
			otrs << " ";
		ots.terminateCommand(false);
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

#ifdef HAVE_LONG_LONG_INT
template otexrowstream & operator<< <unsigned long long>(otexrowstream &,
                                                         unsigned long long);
#endif


template <typename Type>
otexstream & operator<<(otexstream & ots, Type value)
{
	ots.os() << value;
	ots.lastChar(0);
	ots.protectSpace(false);
	ots.terminateCommand(false);
	return ots;
}

template otexstream & operator<< <SetEnc>(otexstream & os, SetEnc);
template otexstream & operator<< <double>(otexstream &, double);
template otexstream & operator<< <int>(otexstream &, int);
template otexstream & operator<< <unsigned int>(otexstream &, unsigned int);
template otexstream & operator<< <unsigned long>(otexstream &, unsigned long);
#ifdef HAVE_LONG_LONG_INT
template otexstream & operator<< <unsigned long long>(otexstream &, unsigned long long);
#endif

} // namespace lyx
