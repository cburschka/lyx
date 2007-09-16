/**
 * \file qt4/KeySymbolFactory.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger & Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/KeySymbol.h"

#include "GuiKeySymbol.h"

#include "qlkey.h"
#include "qt_helpers.h"

#include "debug.h"

#include "support/lstrings.h"
#include "support/environment.h"
#include "support/unicode.h"

#include "Encoding.h"
#include "Language.h"

#include <QKeyEvent>
#include <QKeySequence>
#include <QEvent>
#include <QTextCodec>

#include <map>


namespace lyx {

using std::endl;
using std::string;
using std::map;
using lyx::support::contains;
using lyx::support::getEnv;


KeySymbol * createKeySymbol()
{
	return new GuiKeySymbol;
}


namespace {

char encode(string const & encoding, QString const & str)
{
	typedef map<string, QTextCodec *> EncodingMap;
	EncodingMap encoding_map;

	QTextCodec * codec = 0;

	EncodingMap::const_iterator cit = encoding_map.find(encoding);
	if (cit == encoding_map.end()) {
		LYXERR(Debug::KEY) << "Unrecognised encoding '" << encoding
				   << "'." << endl;
		codec = encoding_map.find("")->second;
	} else {
		codec = cit->second;
	}

	if (!codec) {
		LYXERR(Debug::KEY) << "No codec for encoding '" << encoding
				   << "' found." << endl;
		return 0;
	}

	LYXERR(Debug::KEY) << "Using codec " << fromqstr(codec->name()) << endl;

	if (!codec->canEncode(str)) {
		LYXERR(Debug::KEY) << "Oof. Can't encode the text !" << endl;
		return 0;
	}

	return codec->fromUnicode(str).data()[0];
}

} // anon namespace

GuiKeySymbol::GuiKeySymbol()
	: KeySymbol(), key_(0)
{
}


void GuiKeySymbol::set(QKeyEvent * ev)
{
	key_ = ev->key();
	if (ev->text().isNull()) {
		LYXERR(Debug::KEY) << "keyevent has isNull() text !" << endl;
		text_ = "";
		return;
	}
	text_ = ev->text();
	LYXERR(Debug::KEY) << "Setting key to " << key_ << ", " <<  fromqstr(text_) << endl;
}


void GuiKeySymbol::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
	text_ = toqstr(symbolname);
	LYXERR(Debug::KEY) << "Init key to " << key_ << ", " << fromqstr(text_) << endl;
}


bool GuiKeySymbol::isOK() const
{
	bool const ok(!(text_.isEmpty() && key_ == Qt::Key_unknown));
	LYXERR(Debug::KEY) << "isOK is " << ok << endl;
	return ok;
}


bool GuiKeySymbol::isModifier() const
{
	bool const mod(q_is_modifier(key_));
	LYXERR(Debug::KEY) << "isMod is " << mod << endl;
	return mod;
}


string GuiKeySymbol::getSymbolName() const
{
	string sym(qkey_to_string(key_));

	// e.g. A-Za-z, and others
	if (sym.empty())
		sym = fromqstr(text_);

	return sym;
}


char_type GuiKeySymbol::getUCSEncoded() const
{
	if (text_.isEmpty())
		return 0;

	// UTF16 has a maximum of two characters.
	BOOST_ASSERT(text_.size() <= 2);

	if (lyxerr.debugging() && text_.size() > 1) {
		// We don't know yet how well support the full ucs4 range.
		LYXERR(Debug::KEY) << "GuiKeySymbol::getUCSEncoded()" << endl;
		for (int i = 0; i < text_.size(); ++i) {
			LYXERR(Debug::KEY) << "char " << i << ": "
				<< text_[i].unicode() << endl;
		}
	}

	// Only one UCS4 character at the end.
	docstring ucs4_text = qstring_to_ucs4(text_);
	return ucs4_text[0];
}


docstring const GuiKeySymbol::print(key_modifier::state mod, bool forgui) const
{
	int tmpkey = key_;

	if (mod & key_modifier::shift)
		tmpkey += Qt::SHIFT;
	if (mod & key_modifier::ctrl)
		tmpkey += Qt::CTRL;
	if (mod & key_modifier::alt)
		tmpkey += Qt::ALT;

	QKeySequence seq(tmpkey);

	return qstring_to_ucs4(seq.toString(forgui ? QKeySequence::NativeText
					    : QKeySequence::PortableText));
}


bool GuiKeySymbol::isText() const
{
	if (text_.isEmpty()) {
		LYXERR(Debug::KEY) << "text_ empty, isText() == false" << endl;
		return false;
	}

	return true;
}


bool GuiKeySymbol::operator==(KeySymbol const & ks) const
{
	GuiKeySymbol const & qks = static_cast<GuiKeySymbol const &>(ks);

	// we do not have enough info for a fair comparison, so return
	// false. This works out OK because unknown text from Qt will
	// get inserted anyway after the isText() check
	if (key_ == Qt::Key_unknown || qks.key_ == Qt::Key_unknown)
		return false;

	return key_ == qks.key_;
}


key_modifier::state q_key_state(Qt::KeyboardModifiers state)
{
	key_modifier::state k = key_modifier::none;
	if (state & Qt::ControlModifier)
		k |= key_modifier::ctrl;
	if (state & Qt::ShiftModifier)
		k |= key_modifier::shift;
	if (state & Qt::AltModifier || state & Qt::MetaModifier)
		k |= key_modifier::alt;
	return k;
}

} // namespace lyx
