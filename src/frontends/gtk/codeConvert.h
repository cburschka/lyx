// -*- C++ -*-
/**
 * \file codeConvert.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CODE_CONVERT_H
#define CODE_CONVERT_H

#include <X11/Xft/Xft.h>


inline XftChar32 * wcsToXftChar32StrFast(wchar_t * wcs)
{
	return reinterpret_cast<XftChar32 *>(wcs);
}


inline XftChar32 * wcsToXftChar32StrFast(wchar_t const * wcs)
{
	return reinterpret_cast<XftChar32 *>(const_cast<wchar_t *>(wcs));
}


inline XftChar32 wcToXftChar32(wchar_t wc)
{
	return static_cast<XftChar32>(wc);
}


#endif
