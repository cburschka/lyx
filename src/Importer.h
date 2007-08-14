// -*- C++ -*-
/**
 * \file Importer.h
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

#include <string>
#include <vector>


namespace lyx {

namespace support { class FileName; }

class ErrorList;
class Format;

namespace frontend {
class LyXView;
}

class Importer {
public:
	///
	static bool Import(frontend::LyXView * lv, support::FileName const & filename,
		    std::string const & format, ErrorList & errorList);

	///
	static std::vector<Format const *> const GetImportableFormats();
private:
	///
	static std::vector<std::string> const Loaders();
};

} // namespace lyx

#endif
