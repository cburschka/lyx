// -*- C++ -*-
/**
 * \file importer.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#ifndef IMPORTER_H
#define IMPORTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LString.h"

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
