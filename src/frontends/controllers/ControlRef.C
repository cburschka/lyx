/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlRef.C
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlRef.h"
#include "lyxfunc.h"
#include "buffer.h"
#include "bufferlist.h"
#include "funcrequest.h"

#include "support/filetools.h" // MakeAbsPath, MakeDisplayPath

using std::vector;

extern BufferList bufferlist;


ControlRef::ControlRef(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_REF_INSERT)
{}


vector<string> const ControlRef::getLabelList(string const & name) const
{
	Buffer const * buf = bufferlist.getBuffer(MakeAbsPath(name));
	if (!buf)
		buf = buffer();
	return buf->getLabelList();
}


void ControlRef::gotoRef(string const & ref)
{
	lyxfunc().dispatch(FuncRequest(LFUN_BOOKMARK_SAVE, "0"), false);
	lyxfunc().dispatch(FuncRequest(LFUN_REF_GOTO, ref));
}


void ControlRef::gotoBookmark()
{
	lyxfunc().dispatch(FuncRequest(LFUN_BOOKMARK_GOTO, "0"), false);
}


vector<string> const ControlRef::getBufferList() const
{
	vector<string> buffers = bufferlist.getFileNames();
	for (vector<string>::iterator it = buffers.begin();
	     it != buffers.end(); ++it) {
		*it = MakeDisplayPath(*it);
	}

	return buffers;
}


int ControlRef::getBufferNum() const
{
	vector<string> buffers = bufferlist.getFileNames();
	string const name = buffer()->fileName();
	vector<string>::const_iterator cit =
		find(buffers.begin(), buffers.end(), name);
	if (cit == buffers.end())
		return 0;
	return int(cit - buffers.begin());
}

string const ControlRef::getBufferName(int num) const
{
	return bufferlist.getFileNames()[num];
}
