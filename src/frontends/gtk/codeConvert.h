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


inline FcChar32 * wcsToFcChar32StrFast(wchar_t * wcs)
{
	return reinterpret_cast<FcChar32*>(wcs);
}


inline FcChar32 const * wcsToFcChar32StrFast(wchar_t const * wcs)
{
	return reinterpret_cast<FcChar32 const *>(wcs);
}


inline FcChar32 wcToFcChar32(wchar_t wc)
{
	return static_cast<FcChar32>(wc);
}


#endif
