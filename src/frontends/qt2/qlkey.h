/**
 * \file qlkey.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QLKEY_H
#define QLKEY_H
 
#include <config.h>
#include <qnamespace.h>

#include "LString.h"
 
/**
 * q_to_lkey - convert Qt keypress into LyX
 *
 * Convert the Qt keypress into a string understandable
 * by the LyX core (same as XKeysymToString)
 */
string const q_to_lkey(int lkey) 
{
	switch(lkey) {
	case Qt::Key_Escape: return "Escape";
	case Qt::Key_Tab: return "Tab";
	case Qt::Key_BackSpace: return "BackSpace";
	case Qt::Key_Return: return "Return";
	case Qt::Key_Enter: return "KP_Enter"; // correct ??
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
	case Qt::Key_Prior: return "Prior";
	case Qt::Key_Next: return "Next";
	case Qt::Key_Shift: return "Shift_L";
	case Qt::Key_Control: return "Control_L";
	case Qt::Key_Meta: return "Alt_L"; // correct ?
	case Qt::Key_Alt: return "Alt_L";
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
	case Qt::Key_Colon: return "colon";
	case Qt::Key_Semicolon: return "semicolon";
	case Qt::Key_Less: return "less";
	case Qt::Key_Equal: return "equal";
	case Qt::Key_Greater: return "greater";
	case Qt::Key_Question: return "question";
	case Qt::Key_At: return "at";
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
	case Qt::Key_agrave: return "agrave";
	case Qt::Key_aacute: return "aacute";
	case Qt::Key_acircumflex: return "acircumflex";
	case Qt::Key_atilde: return "atilde";
	case Qt::Key_adiaeresis: return "adiaeresis";
	case Qt::Key_aring: return "aring";
	case Qt::Key_ae: return "ae";
	case Qt::Key_ccedilla: return "ccedilla";
	case Qt::Key_egrave: return "egrave";
	case Qt::Key_eacute: return "eacute";
	case Qt::Key_ecircumflex: return "ecircumflex";
	case Qt::Key_ediaeresis: return "ediaeresis";
	case Qt::Key_igrave: return "igrave";
	case Qt::Key_iacute: return "iacute";
	case Qt::Key_icircumflex: return "icircumflex";
	case Qt::Key_idiaeresis: return "idiaeresis";
	case Qt::Key_eth: return "eth";
	case Qt::Key_ntilde: return "ntilde";
	case Qt::Key_ograve: return "ograve";
	case Qt::Key_oacute: return "oacute";
	case Qt::Key_ocircumflex: return "ocircumflex";
	case Qt::Key_otilde: return "otilde";
	case Qt::Key_odiaeresis: return "odiaeresis";
	case Qt::Key_division: return "division";
	case Qt::Key_oslash: return "oslash";
	case Qt::Key_ugrave: return "ugrave";
	case Qt::Key_uacute: return "uacute";
	case Qt::Key_ucircumflex: return "ucircumflex";
	case Qt::Key_udiaeresis: return "udiaeresis";
	case Qt::Key_yacute: return "yacute";
	case Qt::Key_thorn: return "thorn";
	case Qt::Key_ydiaeresis: return "ydiaeresis";

	// FIXME: these ones I don't know the names of ... help !
	// what's here is basically guesses ...
	case Qt::Key_Super_L: return "Super_L";
	case Qt::Key_Super_R: return "Super_R";
	case Qt::Key_Menu: return "Menu";
	case Qt::Key_Hyper_L: return "Hyper_L";
	case Qt::Key_Hyper_R: return "Hyper_R";
	case Qt::Key_Help: return "Help";
	case Qt::Key_Bar: return "Bar";
	case Qt::Key_Backtab: return "BackTab";

	default:
	case Qt::Key_unknown: return "unknown";
	}
} 

#endif // QLKEY_H
