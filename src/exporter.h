// -*- C++ -*-
/**
 * \file exporter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXPORTER_H
#define EXPORTER_H

#include <vector>
#include "LString.h"

class Buffer;
class Format;

class Exporter {
public:
	///
	static
	bool Export(Buffer * buffer, string const & format,
		    bool put_in_tempdir, string & result_file);
	///
	static
	bool Export(Buffer * buffer, string const & format,
		    bool put_in_tempdir);
	///
	static
	bool Preview(Buffer * buffer, string const & format);
	///
	static
	bool IsExportable(Buffer const & buffer, string const & format);
	///
	static
	std::vector<Format const *> const
	GetExportableFormats(Buffer const & buffer, bool only_viewable);
	///
};
#endif
