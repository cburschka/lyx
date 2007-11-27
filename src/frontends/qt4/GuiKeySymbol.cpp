/**
 * \file qt4/GuiKeySymbol.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger & Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "KeySymbol.h"

#include "qlkey.h"
#include "qt_helpers.h"

#include "debug.h"

#include "Encoding.h"
#include "Language.h"

#include <QKeyEvent>
#include <QKeySequence>
#include <QEvent>
#include <QTextCodec>

#include "boost/assert.hpp"

#include <map>


using std::string;
using std::map;

namespace lyx {

#if 0
static char encode(string const & encoding, QString const & str)
{
	typedef map<string, QTextCodec *> EncodingMap;
	EncodingMap encoding_map;

	QTextCodec * codec = 0;

	EncodingMap::const_iterator cit = encoding_map.find(encoding);
	if (cit == encoding_map.end()) {
		LYXERR(Debug::KEY, "Unrecognised encoding '" << encoding << "'.");
		codec = encoding_map.find("")->second;
	} else {
		codec = cit->second;
	}

	if (!codec) {
		LYXERR(Debug::KEY, "No codec for encoding '" << encoding << "' found.");
		return 0;
	}

	LYXERR(Debug::KEY, "Using codec " << fromqstr(codec->name()));

	if (!codec->canEncode(str)) {
		LYXERR(Debug::KEY, "Oof. Can't encode the text !");
		return 0;
	}

	return codec->fromUnicode(str).data()[0];
}
#endif


void setKeySymbol(KeySymbol * sym, QKeyEvent * ev)
{
	sym->setKey(ev->key());
	if (ev->text().isNull()) {
		LYXERR(Debug::KEY, "keyevent has isNull() text !");
		sym->setText(docstring());
		return;
	}
	LYXERR(Debug::KEY, "Getting key " << ev->key() << ", with text '"
		<< fromqstr(ev->text()) << "'");
	// This is unsafe because ev->text() is the unicode representation of the
	// key, not the name of the key. For example, Ctrl-x and Alt-x produce 
	// different texts.
	sym->setText(qstring_to_ucs4(ev->text()));
	LYXERR(Debug::KEY, "Setting key to " << sym->key() << ", "
		<< to_utf8(sym->text()));
}


void KeySymbol::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
	text_ = from_utf8(symbolname);
	LYXERR(Debug::KEY, "Init key to " << key_ << ", " << to_utf8(text_));
}


bool KeySymbol::isOK() const
{
	bool const ok = !(text_.empty() && key_ == Qt::Key_unknown);
	LYXERR(Debug::KEY, "isOK is " << ok);
	return ok;
}


bool KeySymbol::isModifier() const
{
	bool const mod = q_is_modifier(key_);
	LYXERR(Debug::KEY, "isModifier is " << mod);
	return mod;
}


string KeySymbol::getSymbolName() const
{
	string name = qkey_to_string(key_);

	// others
	if (name.empty())
		name = to_utf8(text_);

	return name;
}


char_type KeySymbol::getUCSEncoded() const
{
	if (text_.empty())
		return 0;

	// UTF16 has a maximum of two characters.
	BOOST_ASSERT(text_.size() <= 2);

	if (lyxerr.debugging() && text_.size() > 1) {
		// We don't know yet how well support the full ucs4 range.
		LYXERR(Debug::KEY, "KeySymbol::getUCSEncoded()");
		for (int i = 0; i != int(text_.size()); ++i)
			LYXERR(Debug::KEY, "char " << i << ": " << int(text_[i]));
	}

	return text_[0];
}


docstring const KeySymbol::print(KeyModifier mod, bool forgui) const
{
	int tmpkey = key_;

	if (mod & ShiftModifier)
		tmpkey += Qt::ShiftModifier;
	if (mod & ControlModifier)
		tmpkey += Qt::ControlModifier;
	if (mod & AltModifier)
		tmpkey += Qt::AltModifier;

	QKeySequence seq(tmpkey);

	return qstring_to_ucs4(seq.toString(forgui ? QKeySequence::NativeText
					    : QKeySequence::PortableText));
}


bool KeySymbol::isText() const
{
	if (!text_.empty())
		return true;
	LYXERR(Debug::KEY, "text_ empty, isText() == false");
	return false;
}


bool KeySymbol::operator==(KeySymbol const & ks) const
{
	// we do not have enough info for a fair comparison, so return
	// false. This works out OK because unknown text from Qt will
	// get inserted anyway after the isText() check
	if (key_ == Qt::Key_unknown || ks.key_ == Qt::Key_unknown)
		return false;
	return key_ == ks.key_;
}


KeyModifier q_key_state(Qt::KeyboardModifiers state)
{
	KeyModifier k = NoModifier;
	if (state & Qt::ControlModifier)
		k |= ControlModifier;
	if (state & Qt::ShiftModifier)
		k |= ShiftModifier;
	if (state & Qt::AltModifier || state & Qt::MetaModifier)
		k |= AltModifier;
	return k;
}

} // namespace lyx
