/**
 * \file MathStream.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathStream.h"

#include "MathFactory.h"
#include "MathData.h"
#include "MathExtern.h"

#include "support/textutils.h"
#include "support/docstring.h"

#include <algorithm>
#include <cstring>
#include <ostream>

using namespace std;

namespace lyx {


//////////////////////////////////////////////////////////////////////


NormalStream & operator<<(NormalStream & ns, MathAtom const & at)
{
	at->normalize(ns);
	return ns;
}


NormalStream & operator<<(NormalStream & ns, MathData const & ar)
{
	normalize(ar, ns);
	return ns;
}


NormalStream & operator<<(NormalStream & ns, docstring const & s)
{
	ns.os() << s;
	return ns;
}


NormalStream & operator<<(NormalStream & ns, const string & s)
{
	ns.os() << from_utf8(s);
	return ns;
}


NormalStream & operator<<(NormalStream & ns, char const * s)
{
	ns.os() << s;
	return ns;
}


NormalStream & operator<<(NormalStream & ns, char c)
{
	ns.os() << c;
	return ns;
}


NormalStream & operator<<(NormalStream & ns, int i)
{
	ns.os() << i;
	return ns;
}



/////////////////////////////////////////////////////////////////


WriteStream & operator<<(WriteStream & ws, docstring const & s)
{
	// Skip leading '\n' if we had already output a newline char
	size_t const first =
		(s.length() > 0 && (s[0] != '\n' || ws.canBreakLine())) ? 0 : 1;

	// Check whether there's something to output
	if (s.length() <= first)
		return ws;

	if (ws.pendingBrace()) {
		ws.os() << '}';
		ws.pendingBrace(false);
		ws.pendingSpace(false);
		ws.textMode(true);
	} else if (ws.pendingSpace()) {
		if (isAlphaASCII(s[first]))
			ws.os() << ' ';
		else if (s[first] == ' ' && ws.textMode())
			ws.os() << '\\';
		ws.pendingSpace(false);
	}
	ws.os() << s.substr(first);
	int lf = 0;
	char_type lastchar = 0;
	docstring::const_iterator dit = s.begin() + first;
	docstring::const_iterator end = s.end();
	for (; dit != end; ++dit) {
		lastchar = *dit;
		if (lastchar == '\n')
			++lf;
	}
	ws.addlines(lf);
	ws.canBreakLine(lastchar != '\n');
	return ws;
}


WriteStream::WriteStream(otexrowstream & os, bool fragile, bool latex,
						 OutputType output, Encoding const * encoding)
	: os_(os), fragile_(fragile), firstitem_(false), latex_(latex),
	  output_(output), pendingspace_(false), pendingbrace_(false),
	  textmode_(false), locked_(0), ascii_(0), canbreakline_(true),
	  line_(0), encoding_(encoding)
{}


WriteStream::WriteStream(otexrowstream & os)
	: os_(os), fragile_(false), firstitem_(false), latex_(false),
	  output_(wsDefault), pendingspace_(false), pendingbrace_(false),
	  textmode_(false), locked_(0), ascii_(0), canbreakline_(true),
	  line_(0), encoding_(0) 
{}


WriteStream::~WriteStream()
{
	if (pendingbrace_)
		os_ << '}';
	else if (pendingspace_)
		os_ << ' ';
}


void WriteStream::addlines(unsigned int n)
{
	line_ += n;
}


void WriteStream::pendingSpace(bool how)
{
	pendingspace_ = how;
}


void WriteStream::pendingBrace(bool brace)
{
	pendingbrace_ = brace;
}


void WriteStream::textMode(bool textmode)
{
	textmode_ = textmode;
}


void WriteStream::lockedMode(bool locked)
{
	locked_ = locked;
}


void WriteStream::asciiOnly(bool ascii)
{
	ascii_ = ascii;
}


void WriteStream::pushRowEntry(TexRow::RowEntry entry)
{
	outer_row_entries_.push_back(entry);
}


void WriteStream::popRowEntry()
{
	if (!outer_row_entries_.empty())
		outer_row_entries_.pop_back();
}


bool WriteStream::startOuterRow()
{
	size_t n = outer_row_entries_.size();
	if (n > 0)
		return texrow().start(outer_row_entries_[n - 1]);
	else
		return false;
}


WriteStream & operator<<(WriteStream & ws, MathAtom const & at)
{
	at->write(ws);
	return ws;
}


WriteStream & operator<<(WriteStream & ws, MathData const & ar)
{
	write(ar, ws);
	return ws;
}


WriteStream & operator<<(WriteStream & ws, char const * s)
{
	ws << from_utf8(s);
	return ws;
}


WriteStream & operator<<(WriteStream & ws, char c)
{
	if (c == '\n' && !ws.canBreakLine())
		return ws;

	if (ws.pendingBrace()) {
		ws.os() << '}';
		ws.pendingBrace(false);
		ws.pendingSpace(false);
		ws.textMode(true);
	} else if (ws.pendingSpace()) {
		if (isAlphaASCII(c))
			ws.os() << ' ';
		else if (c == ' ' && ws.textMode())
			ws.os() << '\\';
		ws.pendingSpace(false);
	}
	ws.os() << c;
	if (c == '\n')
		ws.addlines(1);
	ws.canBreakLine(c != '\n');
	return ws;
}


WriteStream & operator<<(WriteStream & ws, int i)
{
	if (ws.pendingBrace()) {
		ws.os() << '}';
		ws.pendingBrace(false);
		ws.textMode(true);
	}
	ws.os() << i;
	ws.canBreakLine(true);
	return ws;
}


WriteStream & operator<<(WriteStream & ws, unsigned int i)
{
	if (ws.pendingBrace()) {
		ws.os() << '}';
		ws.pendingBrace(false);
		ws.textMode(true);
	}
	ws.os() << i;
	ws.canBreakLine(true);
	return ws;
}


//////////////////////////////////////////////////////////////////////


MathStream::MathStream(odocstream & os)
	: os_(os), tab_(0), line_(0), in_text_(false)
{}


void MathStream::cr()
{
	os() << '\n';
	for (int i = 0; i < tab(); ++i)
		os() << ' ';
}


void MathStream::defer(docstring const & s)
{
	deferred_ << s;
}


void MathStream::defer(string const & s)
{
	deferred_ << from_utf8(s);
}


docstring MathStream::deferred() const
{ 
	return deferred_.str();
}


MathStream & operator<<(MathStream & ms, MathAtom const & at)
{
	at->mathmlize(ms);
	return ms;
}


MathStream & operator<<(MathStream & ms, MathData const & ar)
{
	mathmlize(ar, ms);
	return ms;
}


MathStream & operator<<(MathStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MathStream & operator<<(MathStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MathStream & operator<<(MathStream & ms, char_type c)
{
	ms.os().put(c);
	return ms;
}


MathStream & operator<<(MathStream & ms, MTag const & t)
{
	++ms.tab();
	ms.cr();
	ms.os() << '<' << from_ascii(t.tag_);
	if (!t.attr_.empty())
		ms.os() << " " << from_ascii(t.attr_);
	ms << '>';
	return ms;
}


MathStream & operator<<(MathStream & ms, ETag const & t)
{
	ms.cr();
	if (ms.tab() > 0)
		--ms.tab();
	ms.os() << "</" << from_ascii(t.tag_) << '>';
	return ms;
}


MathStream & operator<<(MathStream & ms, docstring const & s)
{
	ms.os() << s;
	return ms;
}


//////////////////////////////////////////////////////////////////////


HtmlStream::HtmlStream(odocstream & os)
	: os_(os), tab_(0), line_(0), in_text_(false)
{}


void HtmlStream::defer(docstring const & s)
{
	deferred_ << s;
}


void HtmlStream::defer(string const & s)
{
	deferred_ << from_utf8(s);
}


docstring HtmlStream::deferred() const
{ 
	return deferred_.str();
}


HtmlStream & operator<<(HtmlStream & ms, MathAtom const & at)
{
	at->htmlize(ms);
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, MathData const & ar)
{
	htmlize(ar, ms);
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, char_type c)
{
	ms.os().put(c);
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, MTag const & t)
{
	ms.os() << '<' << from_ascii(t.tag_);
	if (!t.attr_.empty())
		ms.os() << " " << from_ascii(t.attr_);
	ms << '>';
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, ETag const & t)
{
	ms.os() << "</" << from_ascii(t.tag_) << '>';
	return ms;
}


HtmlStream & operator<<(HtmlStream & ms, docstring const & s)
{
	ms.os() << s;
	return ms;
}


//////////////////////////////////////////////////////////////////////


SetMode::SetMode(MathStream & os, bool text)
	: os_(os)
{
	was_text_ = os_.inText();
	os_.setTextMode(text);
}


SetMode::~SetMode()
{
	os_.setTextMode(was_text_);
}


//////////////////////////////////////////////////////////////////////


SetHTMLMode::SetHTMLMode(HtmlStream & os, bool text)
	: os_(os)
{
	was_text_ = os_.inText();
	os_.setTextMode(text);
}


SetHTMLMode::~SetHTMLMode()
{
	os_.setTextMode(was_text_);
}


//////////////////////////////////////////////////////////////////////


MapleStream & operator<<(MapleStream & ms, MathAtom const & at)
{
	at->maple(ms);
	return ms;
}


MapleStream & operator<<(MapleStream & ms, MathData const & ar)
{
	maple(ar, ms);
	return ms;
}


MapleStream & operator<<(MapleStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MapleStream & operator<<(MapleStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MapleStream & operator<<(MapleStream & ms, int i)
{
	ms.os() << i;
	return ms;
}


MapleStream & operator<<(MapleStream & ms, char_type c)
{
	ms.os().put(c);
	return ms;
}


MapleStream & operator<<(MapleStream & ms, docstring const & s)
{
	ms.os() << s;
	return ms;
}


//////////////////////////////////////////////////////////////////////


MaximaStream & operator<<(MaximaStream & ms, MathAtom const & at)
{
	at->maxima(ms);
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, MathData const & ar)
{
	maxima(ar, ms);
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, int i)
{
	ms.os() << i;
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, docstring const & s)
{
	ms.os() << s;
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, char_type c)
{
	ms.os().put(c);
	return ms;
}


//////////////////////////////////////////////////////////////////////


MathematicaStream & operator<<(MathematicaStream & ms, MathAtom const & at)
{
	at->mathematica(ms);
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, MathData const & ar)
{
	mathematica(ar, ms);
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, int i)
{
	ms.os() << i;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, docstring const & s)
{
	ms.os() << s;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, char_type c)
{
	ms.os().put(c);
	return ms;
}


//////////////////////////////////////////////////////////////////////


OctaveStream & operator<<(OctaveStream & ns, MathAtom const & at)
{
	at->octave(ns);
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, MathData const & ar)
{
	octave(ar, ns);
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, char const * s)
{
	ns.os() << s;
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, char c)
{
	ns.os() << c;
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, int i)
{
	ns.os() << i;
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, docstring const & s)
{
	ns.os() << s;
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, char_type c)
{
	ns.os().put(c);
	return ns;
}


OctaveStream & operator<<(OctaveStream & os, string const & s)
{
	os.os() << from_utf8(s);
	return os;
}


docstring convertDelimToXMLEscape(docstring const & name)
{
	if (name.size() == 1) {
		char_type const c = name[0];
		if (c == '<')
			return from_ascii("&lt;");
		else if (c == '>')
			return from_ascii("&gt;");
		else
			return name;
	}
	MathWordList const & words = mathedWordList();
	MathWordList::const_iterator it = words.find(name);
	if (it != words.end()) {
		docstring const escape = it->second.xmlname;
		return escape;
	}
	LYXERR0("Unable to find `" << name <<"' in the mathWordList.");
	return name;
}

} // namespace lyx
