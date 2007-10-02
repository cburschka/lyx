/**
 * \file KeySequence.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "KeySequence.h"

#include "gettext.h"
#include "KeyMap.h"
#include "lfuns.h"

#include "frontends/KeySymbol.h"

using std::make_pair;
using std::string;


namespace lyx {

FuncRequest const & KeySequence::addkey(KeySymbol const & key,
	KeyModifier mod, KeyModifier nmod)
{
	// adding a key to a deleted sequence
	// starts a new sequence
	if (deleted_) {
		deleted_ = false;
		sequence.clear();
		modifiers.clear();
	}

	modifiers.push_back(make_pair(mod, nmod));
	sequence.push_back(key);

	if (curmap) {
		return curmap->lookup(key, mod, this);
	}

	static FuncRequest unknown(LFUN_UNKNOWN_ACTION);
	return unknown;
}


size_t KeySequence::parse(string const & s)
{
	if (s.empty())
		return 1;

	size_t i = 0;
	KeyModifier mod = NoModifier;
	KeyModifier nmod = NoModifier;

	while (i < s.length()) {
		if (s[i] == ' ')
			++i;
		if (i >= s.length())
			break;

		if (i + 1 < s.length() && s[i + 1] == '-') {
			switch (s[i]) {
			case 's': case 'S':
				mod |= ShiftModifier;
				i += 2;
				continue;
			case 'c': case 'C':
				mod |= ControlModifier;
				i += 2;
				continue;
			case 'm': case 'M':
				mod |= AltModifier;
				i += 2;
				continue;
			default:
				return i + 1;
			}
		} else if (i + 2 < s.length() && s[i] == '~'
			   && s[i + 2] == '-') {
			switch (s[i + 1]) {
			case 's': case 'S':
				nmod |= ShiftModifier;
				i += 3;
				continue;
			case 'c': case 'C':
				nmod |= ControlModifier;
				i += 3;
				continue;
			case 'm': case 'M':
				nmod |= AltModifier;
				i += 3;
				continue;
			default:
				return i + 2;
			}
		} else {
			string tbuf;
			size_t j = i;
			for (; j < s.length() && s[j] != ' '; ++j)
				tbuf += s[j];    // (!!!check bounds :-)

			KeySymbol key;
			key.init(tbuf);

			if (!key.isOK())
				return j;

			i = j;

			addkey(key, mod, nmod);
			mod = NoModifier;
		}
	}

	// empty sequence?
	if (sequence.size() == 0)
		return 0;

	// everything is fine
	return string::npos;
}


docstring const KeySequence::print(bool forgui) const
{
	docstring buf;

	size_t const length = sequence.size();

	for (size_t i = 0; i != length; ++i) {
		buf += sequence[i].print(modifiers[i].first, forgui);
		// append a blank
		if (i + 1 != length)
			buf += ' ';
	}
	return buf;
}


docstring const KeySequence::printOptions(bool forgui) const
{
	docstring buf = print(forgui);

	if (!curmap)
		return buf;

	buf += _("   options: ");
	buf += curmap->print(forgui);
	return buf;
}


void KeySequence::mark_deleted()
{
	deleted_ = true;
}


void KeySequence::reset()
{
	mark_deleted();
	curmap = stdmap;
}


void KeySequence::clear()
{
	sequence.clear();
	reset();
}


} // namespace lyx
