// -*- C++ -*-
/**
 * \file socktools.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SOCKTOOLS_H
#define SOCKTOOLS_H

#include <string>

namespace lyx {
namespace support {
namespace socktools {

int listen(std::string const &, int);
int accept(int);

} // namespace socktools
} // namespace support
} // namespace lyx

#endif // NOT SOCKTOOLS_H
