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

#include "TexRow.h"


#include "support/debug.h"
#include "support/docstring.h"
#include "support/textutils.h"

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


TeXMathStream & operator<<(TeXMathStream & ws, docstring const & s)
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
		else if (s[first] == '[' && ws.useBraces())
			ws.os() << "{}";
		else if (s[first] == ' ' && ws.textMode())
			ws.os() << '\\';
		ws.pendingSpace(false);
	} else if (ws.useBraces()) {
		if (s[first] == '\'')
			ws.os() << "{}";
		ws.useBraces(false);
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


TeXMathStream::TeXMathStream(otexrowstream & os, bool fragile, bool latex,
                             OutputType output, Encoding const * encoding)
	: os_(os), fragile_(fragile), latex_(latex),
	  output_(output), encoding_(encoding)
{}


TeXMathStream::~TeXMathStream()
{
	if (pendingbrace_)
		os_ << '}';
	else if (pendingspace_)
		os_ << ' ';
}


void TeXMathStream::addlines(unsigned int n)
{
	line_ += n;
}


void TeXMathStream::pendingSpace(bool space)
{
	pendingspace_ = space;
	if (!space)
		usebraces_ = false;
}


void TeXMathStream::useBraces(bool braces)
{
	usebraces_ = braces;
}


void TeXMathStream::pendingBrace(bool brace)
{
	pendingbrace_ = brace;
}


void TeXMathStream::textMode(bool textmode)
{
	textmode_ = textmode;
}


void TeXMathStream::lockedMode(bool locked)
{
	locked_ = locked;
}


void TeXMathStream::asciiOnly(bool ascii)
{
	ascii_ = ascii;
}


Changer TeXMathStream::changeRowEntry(TexRow::RowEntry entry)
{
	return changeVar(row_entry_, entry);
}


bool TeXMathStream::startOuterRow()
{
	if (TexRow::isNone(row_entry_))
		return false;
	return texrow().start(row_entry_);
}


TeXMathStream & operator<<(TeXMathStream & ws, MathAtom const & at)
{
	at->write(ws);
	return ws;
}


TeXMathStream & operator<<(TeXMathStream & ws, MathData const & ar)
{
	write(ar, ws);
	return ws;
}


TeXMathStream & operator<<(TeXMathStream & ws, char const * s)
{
	ws << from_utf8(s);
	return ws;
}


TeXMathStream & operator<<(TeXMathStream & ws, char c)
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
		else if (c == '[' && ws.useBraces())
			ws.os() << "{}";
		else if (c == ' ' && ws.textMode())
			ws.os() << '\\';
		ws.pendingSpace(false);
	} else if (ws.useBraces()) {
		if (c == '\'')
			ws.os() << "{}";
		ws.useBraces(false);
	}
	ws.os() << c;
	if (c == '\n')
		ws.addlines(1);
	ws.canBreakLine(c != '\n');
	return ws;
}


TeXMathStream & operator<<(TeXMathStream & ws, int i)
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


TeXMathStream & operator<<(TeXMathStream & ws, unsigned int i)
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


MathMLStream::MathMLStream(odocstream & os, std::string const & xmlns, bool xmlMode)
	: os_(os), tab_(0), line_(0), in_text_(false), xmlns_(xmlns), xml_mode_(xmlMode)
{}


void MathMLStream::cr()
{
	os() << '\n';
	for (int i = 0; i < tab(); ++i)
		os() << ' ';
}


void MathMLStream::defer(docstring const & s)
{
	deferred_ << s;
}


void MathMLStream::defer(string const & s)
{
	deferred_ << from_utf8(s);
}


docstring MathMLStream::deferred() const
{
	return deferred_.str();
}


MathMLStream & operator<<(MathMLStream & ms, MathAtom const & at)
{
	at->mathmlize(ms);
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, MathData const & ar)
{
	mathmlize(ar, ms);
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, char_type c)
{
	ms.os().put(c);
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, MTag const & t)
{
	++ms.tab();
	ms.cr();
	ms.os() << '<' << from_ascii(ms.namespacedTag(t.tag_));
	if (!t.attr_.empty())
		ms.os() << " " << from_ascii(t.attr_);
	ms << ">";
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, ETag const & t)
{
	ms.cr();
	if (ms.tab() > 0)
		--ms.tab();
	ms.os() << "</" << from_ascii(ms.namespacedTag(t.tag_)) << ">";
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, CTag const & t)
{
	ms.cr();
	ms.os() << "<" << from_ascii(ms.namespacedTag(t.tag_));
    if (!t.attr_.empty())
        ms.os() << " " << from_utf8(t.attr_);
    ms.os() << "/>";
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, docstring const & s)
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


SetMode::SetMode(MathMLStream & ms, bool text)
	: ms_(ms)
{
	was_text_ = ms_.inText();
	ms_.setTextMode(text);
}


SetMode::~SetMode()
{
	ms_.setTextMode(was_text_);
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


docstring convertDelimToXMLEscape(docstring const & name, bool xmlmode)
{
	// For the basic symbols, no difference between XML and HTML.
	if (name.size() == 1) {
		char_type const c = name[0];
		if (c == '<')
			return from_ascii("&lt;");
		else if (c == '>')
			return from_ascii("&gt;");
		else
			return name;
	} else if (name.size() == 2 && name[0] == '\\') {
		char_type const c = name[1];
		if (c == '{')
			return from_ascii("&#123;");
		else if (c == '}')
			return from_ascii("&#125;");
	}
	MathWordList const & words = mathedWordList();
	MathWordList::const_iterator it = words.find(name);
	if (it != words.end()) {
		// Only difference between XML and HTML, based on the contents read by MathFactory.
		docstring const escape = xmlmode ? it->second.xmlname : it->second.htmlname;
		return escape;
	}
	LYXERR0("Unable to find `" << name <<"' in the mathWordList.");
	return name;
}

} // namespace lyx
