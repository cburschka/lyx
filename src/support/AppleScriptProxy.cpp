// -*- C++ -*-
/**
 * \file AppleScriptProxy.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Benjamin Piwowarski
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "AppleScript.h"

#include "DispatchResult.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "LyXAction.h"

#include "frontends/Application.h"

#include "support/docstring.h"
#include "support/debug.h"

#include <stdlib.h>

using namespace std;
using namespace lyx;

extern "C" LyXFunctionResult applescript_execute_command(const char *cmd, const char *arg) {
	LYXERR(Debug::ACTION, "Running command [" << cmd << "] with arguments [" << arg << "]");
	FuncRequest fr(lyxaction.lookupFunc(cmd), from_utf8(arg));
	fr.setOrigin(FuncRequest::LYXSERVER);
	DispatchResult dr;
	theApp()->dispatch(fr, dr);

	string const rval = to_utf8(dr.message());
	char *cstr = (char*) malloc((rval.size()+1)*sizeof(rval[0]));
	strcpy (cstr, rval.c_str());

	// Returns the result
	LyXFunctionResult result;
	result.code = dr.error() ? -1 : 0;
	result.message = cstr;

	return result;
}

