/**
 * \file QLyXKeySym.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Juergen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QLyXKeySym.h"
#include "qlkey.h"
#include "debug.h"
#include "qt_helpers.h"

#include <qevent.h>
#include <qtextcodec.h>

using std::endl;


QLyXKeySym::QLyXKeySym()
	: LyXKeySym(), key_(0)
{
}


void QLyXKeySym::set(QKeyEvent * ev)
{
	key_ = ev->key();
	if (ev->text().isNull()) {
		lyxerr[Debug::KEY] << "keyevent has isNull() text !" << endl;
		text_ = "";
		return;
	}
	text_ = ev->text();
	lyxerr[Debug::KEY] << "Setting key to " << key_ << ", " <<  fromqstr(text_) << endl;
}


void QLyXKeySym::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
	text_ = toqstr(symbolname);
	lyxerr[Debug::KEY] << "Init key to " << key_ << ", " << fromqstr(text_) << endl;
}


bool QLyXKeySym::isOK() const
{
	bool const ok(!(text_.isEmpty() && key_ == Qt::Key_unknown));
	lyxerr[Debug::KEY] << "isOK is " << ok << endl;
	return ok;
}

 
bool QLyXKeySym::isModifier() const
{
	bool const mod(q_is_modifier(key_));
	lyxerr[Debug::KEY] << "isMod is " << mod << endl;
	return mod;
}


string QLyXKeySym::getSymbolName() const
{
	string sym(qkey_to_string(key_));

	if (sym.empty()) {
		lyxerr[Debug::KEY] << "sym empty in getSymbolName()" << endl;
		if (!text_.isEmpty())
			sym = fromqstr(text_);
	}
	lyxerr[Debug::KEY] << "getSymbolName() -> " << sym << endl;
	return sym;
}


char QLyXKeySym::getISOEncoded() const
{
	char const c = fromqstr(text_)[0];
	lyxerr[Debug::KEY] << "ISOEncoded returning value " << int(c) << endl;
	return c;
}


bool QLyXKeySym::isText() const
{
	if (text_.isEmpty()) {
		lyxerr[Debug::KEY] << "text_ empty, isText() == false" << endl;
		return false;
	}

	QChar const c(text_[0]);
	lyxerr[Debug::KEY] << "isText for key " << key_ 
		<< " isPrint is " << c.isPrint() << endl;
	return c.isPrint();
}

 
bool operator==(LyXKeySym const & k1, LyXKeySym const & k2)
{
	// note we ignore text_ here (non-strict ==), because
	// text_ is not filled out by keymap initialisation

	return static_cast<QLyXKeySym const &>(k1).key()
		== static_cast<QLyXKeySym const &>(k2).key();

}
