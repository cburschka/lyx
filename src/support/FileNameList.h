// -*- C++ -*-
/**
 * \file FileNameList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILENAMELIST_H
#define FILENAMELIST_H

#include "support/FileName.h"

#include <vector>

namespace lyx {
namespace support {

/**
 * Class for storing file name list.
 * std::vector can not be forward declared in a simple way. Creating a class solves
 * this problem.
 */
class FileNameList: public std::vector<FileName>
{
};


/**
 * List of filename with additional information. Used by the Bibfiles cache,
 * which needs to store, next to the real filename, the way it was entered
 * in the BibTeX inset (as abspath, relpath or texmf file)
 */
class FileNamePairList: public std::vector<std::pair<docstring, FileName>>
{
};

} // namespace support
} // namespace lyx

#endif // FILENAMELIST_H
