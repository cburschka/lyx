/**
 * \file convert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/counter_reps.h"
#include "support/docstring.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {

char loweralphaCounter(int const n)
{
    if (n < 1 || n > 26)
        return '?';
    return 'a' + n - 1;
}


char alphaCounter(int const n)
{
    if (n < 1 || n > 26)
           return '?';
    return 'A' + n - 1;
}


char hebrewCounter(int const n)
{
    static const char hebrew[22] = {
        '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7', '\xe8',
        '\xe9', '\xeb', '\xec', '\xee', '\xf0', '\xf1', '\xf2', '\xf4', '\xf6',
        '\xf7', '\xf8', '\xf9', '\xfa'
    };

    if (n < 1 || n > 22)
        return '?';
    return hebrew[n - 1];
}


// On the special cases, see http://mathworld.wolfram.com/RomanNumerals.html
// and for a list of roman numerals up to and including 3999, see
// http://www.research.att.com/~njas/sequences/a006968.txt. (Thanks to Joost
// for this info.)
docstring const romanCounter(int const n)
{
    static char const * const ones[9] = {
        "I",   "II",  "III", "IV", "V",
        "VI",  "VII", "VIII", "IX"
    };

    static char const * const tens[9] = {
        "X", "XX", "XXX", "XL", "L",
        "LX", "LXX", "LXXX", "XC"
    };

    static char const * const hunds[9] = {
        "C", "CC", "CCC", "CD", "D",
        "DC", "DCC", "DCCC", "CM"
    };

    if (n >= 1000 || n < 1)
          return from_ascii("??");

    int val = n;
    string roman;
    switch (n) {
    //special cases
    case 900:
        roman = "CM";
        break;
    case 400:
        roman = "CD";
        break;
    default:
        if (val >= 100) {
                int hundreds = val / 100;
                roman = hunds[hundreds - 1];
                val = val % 100;
        }
        if (val >= 10) {
            switch (val) {
            //special case
            case 90:
                roman = roman + "XC";
                val = 0; //skip next
                break;
            default:
                int tensnum = val / 10;
                roman = roman + tens[tensnum - 1];
                val = val % 10;
            } // end switch
        } // end tens
        if (val > 0)
             roman = roman + ones[val -1];
    }
    return from_ascii(roman);
}


docstring const lowerromanCounter(int const n)
{
        return support::lowercase(romanCounter(n));
}


docstring const fnsymbolCounter(int const n)
{
	switch(n) {
	case 1: return docstring(1, 0x002a); //*
	case 2: return docstring(1, 0x2020); // dagger
	case 3: return docstring(1, 0x2021); // double dagger
	case 4: return docstring(1, 0x00A7); // section sign
	case 5: return docstring(1, 0x00B6); // pilcrow sign
	case 6: return docstring(1, 0x2016); // vertical bar
	case 7: return docstring(2, 0x002a); // two *
	case 8: return docstring(2, 0x2020); // two daggers
	case 9: return docstring(2, 0x2021); // two double daggers
	default:
		return from_ascii("?");
	};
}

} // namespace lyx
