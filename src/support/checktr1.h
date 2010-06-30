// -*- C++ -*-
/**
 * \file checktr1.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_CHECKTR1_H
#define LYX_CHECKTR1_H

#ifndef LYX_USE_TR1 // When not set by the build system

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#define LYX_USE_TR1
#endif

#if __GNUC__ == 4 && __GNUC_MINOR__ >= 4
#define LYX_USE_TR1
#endif

#endif


#endif
