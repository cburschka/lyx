// -*- C++ -*-
/**
 * \file FileZipDir.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_FILEZIPDIR_H
#define LYX_FILEZIPDIR_H

#include "support/docstring.h"
#include "support/filetools.h"

#include <vector>
#include <utility>
#include <string>

/// The following functions are implemented in minizip/zipunzip.cpp, and are not in
/// the lyx::support namespace

/// zip several files to a zipfile. In-zip filenames are also specified
bool zipFiles(std::string const & zipfile, std::vector<std::pair<std::string, std::string> > const & files);

/// Unzip a zip file to a directory
bool unzipToDir(std::string const & zipfile, std::string const & path);


#endif
