/**
 * \file QLyXKeySym.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Jürgen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLyXKeySym.h"
#include "qlkey.h"
#include "qt_helpers.h"

#include "debug.h"

#include <qaccel.h>
#include <qevent.h>
#include <qtextcodec.h>

#include <map>
#include "support/lstrings.h"
#include "encoding.h"
#include "language.h"

using std::endl;
using std::string;
using std::map;
using lyx::support::contains;


namespace {

typedef map<string, QTextCodec *> EncodingMap;
EncodingMap encoding_map;

char const encode(string const & encoding, QString const & str)
{
	QTextCodec * codec = 0;

	EncodingMap::const_iterator cit = encoding_map.find(encoding);
	if (cit == encoding_map.end()) {
		if (lyxerr.debugging())
			lyxerr[Debug::KEY] << "Unrecognised encoding "
				<< encoding << endl;
		codec = encoding_map.find("")->second;
	} else {
		codec = cit->second;
	}

	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "Using codec " << fromqstr(codec->name()) << endl;

	if (!codec->canEncode(str)) {
		if (lyxerr.debugging())
			lyxerr[Debug::KEY] << "Oof. Can't encode the text !" << endl;
		return 0;
	}

	QCString tmpstr = codec->fromUnicode(str);
	char const * tmpcstr = tmpstr;
	return tmpcstr[0];
}

}


void initEncodings()
{
	const char * c = QTextCodec::locale();
	string s = c;
	if (contains(c, "UTF") || contains(c, "utf")) 
		lyxerr << "Warning: this system's locale uses Unicode." << endl;

	// strip off any encoding suffix
	string::size_type i = s.find(".");
	s = s.substr(0, i);
										       
	encoding_map["iso8859-1"] = QTextCodec::codecForName("ISO 8859-1");
	encoding_map["iso8859-2"] = QTextCodec::codecForName("ISO 8859-2");
	encoding_map["iso8859-3"] = QTextCodec::codecForName("ISO 8859-3");
	encoding_map["iso8859-4"] = QTextCodec::codecForName("ISO 8859-4");
	encoding_map["iso8859-5"] = QTextCodec::codecForName("ISO 8859-5");
	encoding_map["iso8859-6"] = QTextCodec::codecForName("ISO 8859-6");
	encoding_map["iso8859-7"] = QTextCodec::codecForName("ISO 8859-7");
	encoding_map["iso8859-9"] = QTextCodec::codecForName("ISO 8859-9");
	encoding_map["iso8859-15"] = QTextCodec::codecForName("ISO 8859-15");
	encoding_map["cp1255"] = QTextCodec::codecForName("CP 1255");
	encoding_map["cp1251"] = QTextCodec::codecForName("CP 1251");
	encoding_map["koi8"] = QTextCodec::codecForName("KOI8-R");
	encoding_map["koi8-u"] = QTextCodec::codecForName("KOI8-U");

	// FIXME
	encoding_map["tis620-0"] = 0;
	encoding_map["pt154"] = 0;

	// There are lots more codecs in Qt too ...
	
	// when no document open
	// use the appropriate encoding for the system language
	for (Languages::const_iterator it=languages.begin(); it != languages.end(); ++it) {
		lyxerr << it->second.code() << ":" << it->second.encodingStr() << ":" << it->second.encoding() << endl;
		if (it->second.code() == s)
			{
				s = it->second.encodingStr();
				break;
			}
	}
	lyxerr << "Setting new locale for Qt:" << s << endl;
	QTextCodec * defaultCodec = encoding_map[s];
	encoding_map[""] = defaultCodec;

	QTextCodec::setCodecForCStrings(defaultCodec);

}


QLyXKeySym::QLyXKeySym()
	: LyXKeySym(), key_(0)
{
}


void QLyXKeySym::set(QKeyEvent * ev)
{
	key_ = ev->key();
	if (ev->text().isNull()) {
		if (lyxerr.debugging())
			lyxerr[Debug::KEY] << "keyevent has isNull() text !" << endl;
		text_ = "";
		return;
	}
	text_ = ev->text();
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "Setting key to " << key_ << ", " <<  fromqstr(text_) << endl;
}


void QLyXKeySym::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
	text_ = toqstr(symbolname);
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "Init key to " << key_ << ", " << fromqstr(text_) << endl;
}


bool QLyXKeySym::isOK() const
{
	bool const ok(!(text_.isEmpty() && key_ == Qt::Key_unknown));
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "isOK is " << ok << endl;
	return ok;
}


bool QLyXKeySym::isModifier() const
{
	bool const mod(q_is_modifier(key_));
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "isMod is " << mod << endl;
	return mod;
}


string QLyXKeySym::getSymbolName() const
{
	string sym(qkey_to_string(key_));

	// e.g. A-Za-z, and others
	if (sym.empty())
		sym = fromqstr(text_);

	return sym;
}


char QLyXKeySym::getISOEncoded(string const & encoding) const
{
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "encoding is " << encoding << endl;
	unsigned char const c = encode(encoding, text_);
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "ISOEncoded returning value " << int(c) << endl;
	return c;
}


QString const QLyXKeySym::qprint(key_modifier::state mod) const
{
	int tmpkey = key_;

	if (mod & key_modifier::shift)
		tmpkey += Qt::SHIFT;
	if (mod & key_modifier::ctrl)
		tmpkey += Qt::CTRL;
	if (mod & key_modifier::alt)
		tmpkey += Qt::ALT;

	return QAccel::keyToString(tmpkey);
}


string const QLyXKeySym::print(key_modifier::state mod) const
{
	return fromqstr(qprint(mod));
}


bool QLyXKeySym::isText() const
{
	if (text_.isEmpty()) {
		if (lyxerr.debugging())
			lyxerr[Debug::KEY] << "text_ empty, isText() == false" << endl;
		return false;
	}

	QChar const c(text_[0]);
	if (lyxerr.debugging())
		lyxerr[Debug::KEY] << "isText for key " << key_
			<< " isPrint is " << c.isPrint() << endl;
	return c.isPrint();
}


bool operator==(LyXKeySym const & k1, LyXKeySym const & k2)
{
	QLyXKeySym const & q1(static_cast<QLyXKeySym const &>(k1));
	QLyXKeySym const & q2(static_cast<QLyXKeySym const &>(k2));

	// we do not have enough info for a fair comparison, so return
	// false. This works out OK because unknown text from Qt will
	// get inserted anyway after the isText() check
	if (q1.key() == Qt::Key_unknown || q2.key() == Qt::Key_unknown)
		return false;

	return q1.key() == q2.key();
}
