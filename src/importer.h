// -*- C++ -*-
/**
 * \file importer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef IMPORTER_H
#define IMPORTER_H

#include <vector>
#include "support/std_string.h"

class LyXView;
class Format;

class Importer {
public:
	///
	static
	bool Import(LyXView * lv, string const & filename,
		    string const & format);

	///
	static
	std::vector<Format const *> const GetImportableFormats();
private:
	///
	static
	std::vector<string> const Loaders();
};
#endif
