// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "EPS_Renderer.h"

#include FORMS_H_LOCATION
#include <iostream>
#include <fstream>

#include "support/LAssert.h"
#include "debug.h"

using std::endl;
using std::ios;


EPS_Renderer::EPS_Renderer()
	: Renderer()
{}


bool EPS_Renderer::renderImage()
{
	return false;
}


bool EPS_Renderer::isImageFormatOK(string const & filename) const
{
	std::ifstream is(filename.c_str());

	// The signature of the file without the spaces.
	static const char str[] = "%!PS";
	const char * ptr = str;

	do {
		char c;
		is >> c;

		if (c != *ptr)
			return false;
		
		++ptr;
	} while (*ptr != '\0');

	return true;
}
