// -*- C++ -*-
/**
 * \file qt_i18n.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTI18N_H
#define QTI18N_H

class QString;

namespace lyx {

/**
* qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(char const * str, const char * comment = 0);


} // namespace lyx

#endif // QTI18_H
