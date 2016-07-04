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
#include "GuiApplication.h"

#include "qt_helpers.h"

#include "support/lassert.h"
#include "support/debug.h"

#include "Encoding.h"
#include "Language.h"

#include <QKeyEvent>
#include <QKeySequence>
#include <QEvent>
#include <QTextCodec>

#include <map>
#include <string>

using namespace std;


namespace lyx {

/**
 * Return true if the key event is a modifier.
 */
static bool isModifier(int qkey)
{
	switch (qkey) {
		case Qt::Key_Hyper_L:
		case Qt::Key_Hyper_R:
		case Qt::Key_Super_L:
		case Qt::Key_Super_R:
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Meta:
		case Qt::Key_Alt:
		case Qt::Key_AltGr:
			return true;
	}
	return false;
}


/**
 * Return the numeric Qt Key corresponding to the
 * given symbol name.
 */
static int string_to_qkey(std::string const & str)
{
	// FIX! (Lgb)

	if (str == "Escape") return Qt::Key_Escape;
	if (str == "Tab") return Qt::Key_Tab;
	if (str == "ISO_Left_Tab") return Qt::Key_Backtab;
	if (str == "BackSpace") return Qt::Key_Backspace;
	if (str == "Return") return Qt::Key_Return;
	if (str == "KP_Enter") return Qt::Key_Enter; // correct ?
	if (str == "Insert") return Qt::Key_Insert;
	if (str == "KP_Insert") return Qt::Key_Insert;
	if (str == "Delete") return Qt::Key_Delete;
	if (str == "KP_Delete") return Qt::Key_Delete;
	if (str == "Pause") return Qt::Key_Pause;
	if (str == "Print") return Qt::Key_Print;
	if (str == "Sys_Req") return Qt::Key_SysReq;
	if (str == "Home") return Qt::Key_Home;
	if (str == "End") return Qt::Key_End;
	if (str == "Left") return Qt::Key_Left;
	if (str == "Up") return Qt::Key_Up;
	if (str == "Right") return Qt::Key_Right;
	if (str == "Down") return Qt::Key_Down;
	if (str == "Prior") return Qt::Key_PageUp;
	if (str == "Next") return Qt::Key_PageDown;
	if (str == "KP_Home") return Qt::Key_Home;
	if (str == "KP_End") return Qt::Key_End;
	if (str == "KP_Left") return Qt::Key_Left;
	if (str == "KP_Up") return Qt::Key_Up;
	if (str == "KP_Right") return Qt::Key_Right;
	if (str == "KP_Down") return Qt::Key_Down;
	if (str == "KP_Prior") return Qt::Key_PageUp;
	if (str == "KP_Next") return Qt::Key_PageDown;
	if (str == "Shift_L") return Qt::Key_Shift;
	if (str == "Control_L") return Qt::Key_Control;
	if (str == "Alt_L") return Qt::Key_Meta; // correct ?
	if (str == "Alt_R") return Qt::Key_Alt;
	if (str == "Caps_Lock") return Qt::Key_CapsLock;
	if (str == "Num_Lock") return Qt::Key_NumLock;
	if (str == "Scroll_Lock") return Qt::Key_ScrollLock;
	if (str == "F1") return Qt::Key_F1;
	if (str == "F2") return Qt::Key_F2;
	if (str == "F3") return Qt::Key_F3;
	if (str == "F4") return Qt::Key_F4;
	if (str == "F5") return Qt::Key_F5;
	if (str == "F6") return Qt::Key_F6;
	if (str == "F7") return Qt::Key_F7;
	if (str == "F8") return Qt::Key_F8;
	if (str == "F9") return Qt::Key_F9;
	if (str == "F10") return Qt::Key_F10;
	if (str == "F11") return Qt::Key_F11;
	if (str == "F12") return Qt::Key_F12;
	if (str == "F13") return Qt::Key_F13;
	if (str == "F14") return Qt::Key_F14;
	if (str == "F15") return Qt::Key_F15;
	if (str == "F16") return Qt::Key_F16;
	if (str == "F17") return Qt::Key_F17;
	if (str == "F18") return Qt::Key_F18;
	if (str == "F19") return Qt::Key_F19;
	if (str == "F20") return Qt::Key_F20;
	if (str == "F21") return Qt::Key_F21;
	if (str == "F22") return Qt::Key_F22;
	if (str == "F23") return Qt::Key_F23;
	if (str == "F24") return Qt::Key_F24;
	if (str == "F25") return Qt::Key_F25;
	if (str == "F26") return Qt::Key_F26;
	if (str == "F27") return Qt::Key_F27;
	if (str == "F28") return Qt::Key_F28;
	if (str == "F29") return Qt::Key_F29;
	if (str == "F30") return Qt::Key_F30;
	if (str == "F31") return Qt::Key_F31;
	if (str == "F32") return Qt::Key_F32;
	if (str == "F33") return Qt::Key_F33;
	if (str == "F34") return Qt::Key_F34;
	if (str == "F35") return Qt::Key_F35;
	if (str == "0") return Qt::Key_0;
	if (str == "1") return Qt::Key_1;
	if (str == "2") return Qt::Key_2;
	if (str == "3") return Qt::Key_3;
	if (str == "4") return Qt::Key_4;
	if (str == "5") return Qt::Key_5;
	if (str == "6") return Qt::Key_6;
	if (str == "7") return Qt::Key_7;
	if (str == "8") return Qt::Key_8;
	if (str == "9") return Qt::Key_9;
	if (str == "KP_0") return Qt::Key_0;
	if (str == "KP_1") return Qt::Key_1;
	if (str == "KP_2") return Qt::Key_2;
	if (str == "KP_3") return Qt::Key_3;
	if (str == "KP_4") return Qt::Key_4;
	if (str == "KP_5") return Qt::Key_5;
	if (str == "KP_6") return Qt::Key_6;
	if (str == "KP_7") return Qt::Key_7;
	if (str == "KP_8") return Qt::Key_8;
	if (str == "KP_9") return Qt::Key_9;
	if (str == "colon") return Qt::Key_Colon;
	if (str == "semicolon") return Qt::Key_Semicolon;
	if (str == "less") return Qt::Key_Less;
	if (str == "equal") return Qt::Key_Equal;
	if (str == "greater") return Qt::Key_Greater;
	if (str == "question") return Qt::Key_Question;
	if (str == "at") return Qt::Key_At;
	if (str == "A") return Qt::Key_A;
	if (str == "B") return Qt::Key_B;
	if (str == "C") return Qt::Key_C;
	if (str == "D") return Qt::Key_D;
	if (str == "E") return Qt::Key_E;
	if (str == "F") return Qt::Key_F;
	if (str == "G") return Qt::Key_G;
	if (str == "H") return Qt::Key_H;
	if (str == "I") return Qt::Key_I;
	if (str == "J") return Qt::Key_J;
	if (str == "K") return Qt::Key_K;
	if (str == "L") return Qt::Key_L;
	if (str == "M") return Qt::Key_M;
	if (str == "N") return Qt::Key_N;
	if (str == "O") return Qt::Key_O;
	if (str == "P") return Qt::Key_P;
	if (str == "Q") return Qt::Key_Q;
	if (str == "R") return Qt::Key_R;
	if (str == "S") return Qt::Key_S;
	if (str == "T") return Qt::Key_T;
	if (str == "U") return Qt::Key_U;
	if (str == "V") return Qt::Key_V;
	if (str == "W") return Qt::Key_W;
	if (str == "X") return Qt::Key_X;
	if (str == "Y") return Qt::Key_Y;
	if (str == "Z") return Qt::Key_Z;
	if (str == "a") return Qt::Key_A;
	if (str == "b") return Qt::Key_B;
	if (str == "c") return Qt::Key_C;
	if (str == "d") return Qt::Key_D;
	if (str == "e") return Qt::Key_E;
	if (str == "f") return Qt::Key_F;
	if (str == "g") return Qt::Key_G;
	if (str == "h") return Qt::Key_H;
	if (str == "i") return Qt::Key_I;
	if (str == "j") return Qt::Key_J;
	if (str == "k") return Qt::Key_K;
	if (str == "l") return Qt::Key_L;
	if (str == "m") return Qt::Key_M;
	if (str == "n") return Qt::Key_N;
	if (str == "o") return Qt::Key_O;
	if (str == "p") return Qt::Key_P;
	if (str == "q") return Qt::Key_Q;
	if (str == "r") return Qt::Key_R;
	if (str == "s") return Qt::Key_S;
	if (str == "t") return Qt::Key_T;
	if (str == "u") return Qt::Key_U;
	if (str == "v") return Qt::Key_V;
	if (str == "w") return Qt::Key_W;
	if (str == "x") return Qt::Key_X;
	if (str == "y") return Qt::Key_Y;
	if (str == "z") return Qt::Key_Z;
	if (str == "bracketleft") return Qt::Key_BracketLeft;
	if (str == "backslash") return Qt::Key_Backslash;
	if (str == "bracketright") return Qt::Key_BracketRight;
	if (str == "bar") return Qt::Key_Bar;
	if (str == "underscore") return Qt::Key_Underscore;
	if (str == "space") return Qt::Key_Space;
	if (str == "parenleft") return Qt::Key_ParenLeft;
	if (str == "parenright") return Qt::Key_ParenRight;
	if (str == "quotedbl") return Qt::Key_QuoteDbl;
	if (str == "quoteright") return Qt::Key_Apostrophe;
	if (str == "quoteleft") return Qt::Key_QuoteLeft;
	if (str == "exclam") return Qt::Key_Exclam;
	if (str == "numbersign") return Qt::Key_NumberSign;
	if (str == "asciicircum") return Qt::Key_AsciiCircum;
	if (str == "dollar") return Qt::Key_Dollar;
	if (str == "percent") return Qt::Key_Percent;
	if (str == "ampersand") return Qt::Key_Ampersand;
	if (str == "asterisk") return Qt::Key_Asterisk;
	if (str == "KP_Multiply") return Qt::Key_Asterisk;
	if (str == "apostrophe") return Qt::Key_Apostrophe;
	if (str == "plus") return Qt::Key_Plus;
	if (str == "KP_Add") return Qt::Key_Plus;
	if (str == "minus") return Qt::Key_Minus;
	if (str == "KP_Subtract") return Qt::Key_Minus;
	if (str == "comma") return Qt::Key_Comma;
	if (str == "period") return Qt::Key_Period;
	if (str == "KP_Decimal") return Qt::Key_Period;
	if (str == "slash") return Qt::Key_Slash;
	if (str == "KP_Divide") return Qt::Key_Slash;
	if (str == "asciitilde") return Qt::Key_AsciiTilde;
	if (str == "braceleft") return Qt::Key_BraceLeft;
	if (str == "braceright") return Qt::Key_BraceRight;
	if (str == "grave") return Qt::Key_QuoteLeft; // ???
	if (str == "notsign") return Qt::Key_notsign;
	if (str == "nobreakspace") return Qt::Key_nobreakspace;
	if (str == "exclamdown") return Qt::Key_exclamdown;
	if (str == "cent") return Qt::Key_cent;
	if (str == "sterling") return Qt::Key_sterling;
	if (str == "currency") return Qt::Key_currency;
	if (str == "yen") return Qt::Key_yen;
	if (str == "brokenbar") return Qt::Key_brokenbar;
	if (str == "section") return Qt::Key_section;
	if (str == "diaeresis") return Qt::Key_diaeresis;
	if (str == "copyright") return Qt::Key_copyright;
	if (str == "ordfeminine") return Qt::Key_ordfeminine;
	if (str == "guillemotleft") return Qt::Key_guillemotleft;
	if (str == "hyphen") return Qt::Key_hyphen;
	if (str == "registered") return Qt::Key_registered;
	if (str == "macron") return Qt::Key_macron;
	if (str == "degree") return Qt::Key_degree;
	if (str == "plusminus") return Qt::Key_plusminus;
	if (str == "twosuperior") return Qt::Key_twosuperior;
	if (str == "threesuperior") return Qt::Key_threesuperior;
	if (str == "acute") return Qt::Key_acute;
	if (str == "mu") return Qt::Key_mu;
	if (str == "paragraph") return Qt::Key_paragraph;
	if (str == "periodcentered") return Qt::Key_periodcentered;
	if (str == "cedilla") return Qt::Key_cedilla;
	if (str == "onesuperior") return Qt::Key_onesuperior;
	if (str == "masculine") return Qt::Key_masculine;
	if (str == "guillemotright") return Qt::Key_guillemotright;
	if (str == "onequarter") return Qt::Key_onequarter;
	if (str == "onehalf") return Qt::Key_onehalf;
	if (str == "threequarters") return Qt::Key_threequarters;
	if (str == "questiondown") return Qt::Key_questiondown;
	if (str == "Agrave") return Qt::Key_Agrave;
	if (str == "Aacute") return Qt::Key_Aacute;
	if (str == "Acircumflex") return Qt::Key_Acircumflex;
	if (str == "Atilde") return Qt::Key_Atilde;
	if (str == "Adiaeresis") return Qt::Key_Adiaeresis;
	if (str == "Aring") return Qt::Key_Aring;
	if (str == "AE") return Qt::Key_AE;
	if (str == "Ccedilla") return Qt::Key_Ccedilla;
	if (str == "Egrave") return Qt::Key_Egrave;
	if (str == "Eacute") return Qt::Key_Eacute;
	if (str == "Ecircumflex") return Qt::Key_Ecircumflex;
	if (str == "Ediaeresis") return Qt::Key_Ediaeresis;
	if (str == "Igrave") return Qt::Key_Igrave;
	if (str == "Iacute") return Qt::Key_Iacute;
	if (str == "Icircumflex") return Qt::Key_Icircumflex;
	if (str == "Idiaeresis") return Qt::Key_Idiaeresis;
	if (str == "ETH") return Qt::Key_ETH;
	if (str == "Ntilde") return Qt::Key_Ntilde;
	if (str == "Ograve") return Qt::Key_Ograve;
	if (str == "Oacute") return Qt::Key_Oacute;
	if (str == "Ocircumflex") return Qt::Key_Ocircumflex;
	if (str == "Otilde") return Qt::Key_Otilde;
	if (str == "Odiaeresis") return Qt::Key_Odiaeresis;
	if (str == "multiply") return Qt::Key_multiply;
	if (str == "Ooblique") return Qt::Key_Ooblique;
	if (str == "Ugrave") return Qt::Key_Ugrave;
	if (str == "Uacute") return Qt::Key_Uacute;
	if (str == "Ucircumflex") return Qt::Key_Ucircumflex;
	if (str == "Udiaeresis") return Qt::Key_Udiaeresis;
	if (str == "Yacute") return Qt::Key_Yacute;
	if (str == "THORN") return Qt::Key_THORN;
	if (str == "ssharp") return Qt::Key_ssharp;
	if (str == "agrave") return Qt::Key_Agrave;
	if (str == "aacute") return Qt::Key_Aacute;
	if (str == "acircumflex") return Qt::Key_Acircumflex;
	if (str == "atilde") return Qt::Key_Atilde;
	if (str == "adiaeresis") return Qt::Key_Adiaeresis;
	if (str == "aring") return Qt::Key_Aring;
	if (str == "ae") return Qt::Key_AE;
	if (str == "ccedilla") return Qt::Key_Ccedilla;
	if (str == "egrave") return Qt::Key_Egrave;
	if (str == "eacute") return Qt::Key_Eacute;
	if (str == "ecircumflex") return Qt::Key_Ecircumflex;
	if (str == "ediaeresis") return Qt::Key_Ediaeresis;
	if (str == "igrave") return Qt::Key_Igrave;
	if (str == "iacute") return Qt::Key_Iacute;
	if (str == "icircumflex") return Qt::Key_Icircumflex;
	if (str == "idiaeresis") return Qt::Key_Idiaeresis;
	if (str == "eth") return Qt::Key_ETH;
	if (str == "ntilde") return Qt::Key_Ntilde;
	if (str == "ograve") return Qt::Key_Ograve;
	if (str == "oacute") return Qt::Key_Oacute;
	if (str == "ocircumflex") return Qt::Key_Ocircumflex;
	if (str == "otilde") return Qt::Key_Otilde;
	if (str == "odiaeresis") return Qt::Key_Odiaeresis;
	if (str == "division") return Qt::Key_division;
	if (str == "oslash") return Qt::Key_Ooblique;
	if (str == "ugrave") return Qt::Key_Ugrave;
	if (str == "uacute") return Qt::Key_Uacute;
	if (str == "ucircumflex") return Qt::Key_Ucircumflex;
	if (str == "udiaeresis") return Qt::Key_Udiaeresis;
	if (str == "yacute") return Qt::Key_Yacute;
	if (str == "thorn") return Qt::Key_THORN;
	if (str == "ydiaeresis") return Qt::Key_ydiaeresis;
	if (str == "Dead_Caron") return Qt::Key_Dead_Caron;

	// FIXME, correct for all these ?
	if (str == "Super_L") return Qt::Key_Super_L;
	if (str == "Super_R") return Qt::Key_Super_R;
	if (str == "Menu") return Qt::Key_Menu;
	if (str == "Hyper_L") return Qt::Key_Hyper_L;
	if (str == "Hyper_R") return Qt::Key_Hyper_R;
	if (str == "Help") return Qt::Key_Help;
	if (str == "BackTab") return Qt::Key_Backtab;

	return Qt::Key_unknown;
}


/**
 * qkey_to_string - convert Qt keypress into LyX
 *
 * Convert the Qt keypress into a string understandable
 * by the LyX core (same as XKeysymToString).
 */
static std::string const qkey_to_string(int lkey)
{
	switch (lkey) {
	case Qt::Key_0: return "0";
	case Qt::Key_1: return "1";
	case Qt::Key_2: return "2";
	case Qt::Key_3: return "3";
	case Qt::Key_4: return "4";
	case Qt::Key_5: return "5";
	case Qt::Key_6: return "6";
	case Qt::Key_7: return "7";
	case Qt::Key_8: return "8";
	case Qt::Key_9: return "9";

	case Qt::Key_A: return "a";
	case Qt::Key_B: return "b";
	case Qt::Key_C: return "c";
	case Qt::Key_D: return "d";
	case Qt::Key_E: return "e";
	case Qt::Key_F: return "f";
	case Qt::Key_G: return "g";
	case Qt::Key_H: return "h";
	case Qt::Key_I: return "i";
	case Qt::Key_J: return "j";
	case Qt::Key_K: return "k";
	case Qt::Key_L: return "l";
	case Qt::Key_M: return "m";
	case Qt::Key_N: return "n";
	case Qt::Key_O: return "o";
	case Qt::Key_P: return "p";
	case Qt::Key_Q: return "q";
	case Qt::Key_R: return "r";
	case Qt::Key_S: return "s";
	case Qt::Key_T: return "t";
	case Qt::Key_U: return "u";
	case Qt::Key_V: return "v";
	case Qt::Key_W: return "w";
	case Qt::Key_X: return "x";
	case Qt::Key_Y: return "y";
	case Qt::Key_Z: return "z";

	case Qt::Key_Return: return "Return";
	case Qt::Key_Escape: return "Escape";
	case Qt::Key_Tab: return "Tab";
	case Qt::Key_Backspace: return "BackSpace";
	case Qt::Key_Insert: return "Insert";
	case Qt::Key_Delete: return "Delete";
	case Qt::Key_Pause: return "Pause";
	case Qt::Key_Print: return "Print";
	case Qt::Key_SysReq: return "Sys_Req";
	case Qt::Key_Home: return "Home";
	case Qt::Key_End: return "End";
	case Qt::Key_Left: return "Left";
	case Qt::Key_Up: return "Up";
	case Qt::Key_Right: return "Right";
	case Qt::Key_Down: return "Down";
	case Qt::Key_PageUp: return "Prior";
	case Qt::Key_PageDown: return "Next";
	case Qt::Key_Shift: return "Shift_L";
	case Qt::Key_Control: return "Control_L";
	case Qt::Key_Meta: return "Alt_L"; // correct ?
	case Qt::Key_Alt: return "Alt_R";
	case Qt::Key_CapsLock: return "Caps_Lock";
	case Qt::Key_NumLock: return "Num_Lock";
	case Qt::Key_ScrollLock: return "Scroll_Lock";
	case Qt::Key_F1: return "F1";
	case Qt::Key_F2: return "F2";
	case Qt::Key_F3: return "F3";
	case Qt::Key_F4: return "F4";
	case Qt::Key_F5: return "F5";
	case Qt::Key_F6: return "F6";
	case Qt::Key_F7: return "F7";
	case Qt::Key_F8: return "F8";
	case Qt::Key_F9: return "F9";
	case Qt::Key_F10: return "F10";
	case Qt::Key_F11: return "F11";
	case Qt::Key_F12: return "F12";
	case Qt::Key_F13: return "F13";
	case Qt::Key_F14: return "F14";
	case Qt::Key_F15: return "F15";
	case Qt::Key_F16: return "F16";
	case Qt::Key_F17: return "F17";
	case Qt::Key_F18: return "F18";
	case Qt::Key_F19: return "F19";
	case Qt::Key_F20: return "F20";
	case Qt::Key_F21: return "F21";
	case Qt::Key_F22: return "F22";
	case Qt::Key_F23: return "F23";
	case Qt::Key_F24: return "F24";
	case Qt::Key_F25: return "F25";
	case Qt::Key_F26: return "F26";
	case Qt::Key_F27: return "F27";
	case Qt::Key_F28: return "F28";
	case Qt::Key_F29: return "F29";
	case Qt::Key_F30: return "F30";
	case Qt::Key_F31: return "F31";
	case Qt::Key_F32: return "F32";
	case Qt::Key_F33: return "F33";
	case Qt::Key_F34: return "F34";
	case Qt::Key_F35: return "F35";
	case Qt::Key_Colon: return "colon";
	case Qt::Key_Semicolon: return "semicolon";
	case Qt::Key_Less: return "less";
	case Qt::Key_Equal: return "equal";
	case Qt::Key_Greater: return "greater";
	case Qt::Key_Question: return "question";
	case Qt::Key_At: return "at";
	case Qt::Key_BracketLeft: return "bracketleft";
	case Qt::Key_Backslash: return "backslash";
	case Qt::Key_BracketRight: return "bracketright";
	case Qt::Key_Underscore: return "underscore";
	case Qt::Key_Space: return "space";
	case Qt::Key_ParenLeft: return "parenleft";
	case Qt::Key_ParenRight: return "parenright";
	case Qt::Key_QuoteDbl: return "quotedbl";
	case Qt::Key_Exclam: return "exclam";
	case Qt::Key_NumberSign: return "numbersign";
	case Qt::Key_AsciiCircum: return "asciicircum";
	case Qt::Key_Dollar: return "dollar";
	case Qt::Key_Percent: return "percent";
	case Qt::Key_Ampersand: return "ampersand";
	case Qt::Key_Asterisk: return "asterisk";
	case Qt::Key_Apostrophe: return "apostrophe";
	case Qt::Key_Plus: return "plus";
	case Qt::Key_Minus: return "minus";
	case Qt::Key_Comma: return "comma";
	case Qt::Key_Period: return "period";
	case Qt::Key_Slash: return "slash";
	case Qt::Key_AsciiTilde: return "asciitilde";
	case Qt::Key_BraceLeft: return "braceleft";
	case Qt::Key_BraceRight: return "braceright";
	case Qt::Key_QuoteLeft: return "grave"; // ???
	case Qt::Key_notsign: return "notsign";
	case Qt::Key_nobreakspace: return "nobreakspace";
	case Qt::Key_exclamdown: return "exclamdown";
	case Qt::Key_cent: return "cent";
	case Qt::Key_sterling: return "sterling";
	case Qt::Key_currency: return "currency";
	case Qt::Key_yen: return "yen";
	case Qt::Key_brokenbar: return "brokenbar";
	case Qt::Key_section: return "section";
	case Qt::Key_diaeresis: return "diaeresis";
	case Qt::Key_copyright: return "copyright";
	case Qt::Key_ordfeminine: return "ordfeminine";
	case Qt::Key_guillemotleft: return "guillemotleft";
	case Qt::Key_hyphen: return "hyphen";
	case Qt::Key_registered: return "registered";
	case Qt::Key_macron: return "macron";
	case Qt::Key_degree: return "degree";
	case Qt::Key_plusminus: return "plusminus";
	case Qt::Key_twosuperior: return "twosuperior";
	case Qt::Key_threesuperior: return "threesuperior";
	case Qt::Key_acute: return "acute";
	case Qt::Key_mu: return "mu";
	case Qt::Key_paragraph: return "paragraph";
	case Qt::Key_periodcentered: return "periodcentered";
	case Qt::Key_cedilla: return "cedilla";
	case Qt::Key_onesuperior: return "onesuperior";
	case Qt::Key_masculine: return "masculine";
	case Qt::Key_guillemotright: return "guillemotright";
	case Qt::Key_onequarter: return "onequarter";
	case Qt::Key_onehalf: return "onehalf";
	case Qt::Key_threequarters: return "threequarters";
	case Qt::Key_questiondown: return "questiondown";
	case Qt::Key_Agrave: return "Agrave";
	case Qt::Key_Aacute: return "Aacute";
	case Qt::Key_Acircumflex: return "Acircumflex";
	case Qt::Key_Atilde: return "Atilde";
	case Qt::Key_Adiaeresis: return "Adiaeresis";
	case Qt::Key_Aring: return "Aring";
	case Qt::Key_AE: return "AE";
	case Qt::Key_Ccedilla: return "Ccedilla";
	case Qt::Key_Egrave: return "Egrave";
	case Qt::Key_Eacute: return "Eacute";
	case Qt::Key_Ecircumflex: return "Ecircumflex";
	case Qt::Key_Ediaeresis: return "Ediaeresis";
	case Qt::Key_Igrave: return "Igrave";
	case Qt::Key_Iacute: return "Iacute";
	case Qt::Key_Icircumflex: return "Icircumflex";
	case Qt::Key_Idiaeresis: return "Idiaeresis";
	case Qt::Key_ETH: return "ETH";
	case Qt::Key_Ntilde: return "Ntilde";
	case Qt::Key_Ograve: return "Ograve";
	case Qt::Key_Oacute: return "Oacute";
	case Qt::Key_Ocircumflex: return "Ocircumflex";
	case Qt::Key_Otilde: return "Otilde";
	case Qt::Key_Odiaeresis: return "Odiaeresis";
	case Qt::Key_multiply: return "multiply";
	case Qt::Key_Ooblique: return "Ooblique";
	case Qt::Key_Ugrave: return "Ugrave";
	case Qt::Key_Uacute: return "Uacute";
	case Qt::Key_Ucircumflex: return "Ucircumflex";
	case Qt::Key_Udiaeresis: return "Udiaeresis";
	case Qt::Key_Yacute: return "Yacute";
	case Qt::Key_THORN: return "THORN";
	case Qt::Key_ssharp: return "ssharp";
	case Qt::Key_ydiaeresis: return "ydiaeresis";
	case Qt::Key_Bar: return "bar";
	case Qt::Key_Dead_Caron: return "Dead_Caron";

	// FIXME: these ones I don't know the names of ... help !
	// what's here is basically guesses ...
	case Qt::Key_Super_L: return "Super_L";
	case Qt::Key_Super_R: return "Super_R";
	case Qt::Key_Menu: return "Menu";
	case Qt::Key_Hyper_L: return "Hyper_L";
	case Qt::Key_Hyper_R: return "Hyper_R";
	case Qt::Key_Help: return "Help";
	case Qt::Key_Backtab: return "BackTab";

	default:
	case Qt::Key_unknown: return "";
	}
}


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

	LYXERR(Debug::KEY, "Using codec " << codec->name());

	if (!codec->canEncode(str)) {
		LYXERR(Debug::KEY, "Oof. Can't encode the text !");
		return 0;
	}

	return codec->fromUnicode(str).data()[0];
}
#endif


void setKeySymbol(KeySymbol * sym, QKeyEvent const * ev)
{
	sym->setKey(ev->key());
	if (ev->text().isNull()) {
		LYXERR(Debug::KEY, "keyevent has isNull() text !");
		sym->setText(docstring());
		return;
	}
	LYXERR(Debug::KEY, "Getting key " << ev->key() << ", with text '"
		<< ev->text() << "'");
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
	bool const ok = !(text_.empty() && qkey_to_string(key_).empty());
	LYXERR(Debug::KEY, "isOK is " << ok);
	return ok;
}


bool KeySymbol::isModifier() const
{
	bool const mod = lyx::isModifier(key_);
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
	LASSERT(text_.size() <= 2, return 0);

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

	if (mod & ShiftModifier && !(tmpkey == Qt::Key_Shift))
		tmpkey += Qt::ShiftModifier;
	if (mod & ControlModifier && !(tmpkey == Qt::Key_Control))
		tmpkey += Qt::ControlModifier;
	if (mod & AltModifier && !(tmpkey == Qt::Key_Alt))
		tmpkey += Qt::AltModifier;
	if (mod & MetaModifier && !(tmpkey == Qt::Key_Meta))
		tmpkey += Qt::MetaModifier;

	QKeySequence seq(tmpkey);
	QString str;

	if (forgui)
		str = seq.toString(QKeySequence::NativeText);
	else {
#ifdef Q_OS_MAC
		// Qt/Mac does not use Command and friends in the
		// portable case, but the windows-like Control+x (bug 5421).
		str = seq.toString(QKeySequence::NativeText);
		str.replace(QChar(0x21E7), qt_("Shift-"));
		str.replace(QChar(0x2303), qt_("Control-"));
		str.replace(QChar(0x2325), qt_("Option-"));
		str.replace(QChar(0x2318), qt_("Command-"));
#else
		str = seq.toString(QKeySequence::PortableText);	
#endif
	}

	return qstring_to_ucs4(str);
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
#if defined(Q_OS_MAC) && QT_VERSION > 0x050000
	/// Additional check for Control and Meta modifier swap state.
	/// Starting with Qt 5 the modifiers aren't reported correctly.
	/// Until this is fixed a correction is required.
	const bool dontSwapCtrlAndMeta =
		frontend::theGuiApp()->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta);
#else
	const bool dontSwapCtrlAndMeta = false;
#endif

	if (state & (dontSwapCtrlAndMeta ? Qt::MetaModifier : Qt::ControlModifier))
		k |= ControlModifier;
	if (state & Qt::ShiftModifier)
		k |= ShiftModifier;
	if (state & Qt::AltModifier)
		k |= AltModifier;
#if defined(USE_MACOSX_PACKAGING) || defined(USE_META_KEYBINDING)
	if (state & (dontSwapCtrlAndMeta ? Qt::ControlModifier : Qt::MetaModifier))
		k |= MetaModifier;
#else
	if (state & Qt::MetaModifier)
		k |= AltModifier;
#endif
	return k;
}

} // namespace lyx
