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
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlRef.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"
#include "bufferlist.h"
#include "support/filetools.h" // MakeAbsPath, MakeDisplayPath

using SigC::slot;
using std::vector;
using std::find;

extern BufferList bufferlist;

ControlRef::ControlRef(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_REF_INSERT)
{
	d_.showRef.connect(slot(this, &ControlRef::showInset));
	d_.createRef.connect(slot(this, &ControlRef::createInset));
}


vector<string> const ControlRef::getLabelList(string const & name) const
{
	Buffer * buffer = bufferlist.getBuffer(MakeAbsPath(name));
	if (!buffer)
		buffer = lv_.buffer();
	return buffer->getLabelList();
}


void ControlRef::gotoRef(string const & ref) const
{
	lv_.getLyXFunc()->dispatch(LFUN_BOOKMARK_SAVE, "0");
	lv_.getLyXFunc()->dispatch(LFUN_REF_GOTO, ref);
}


void ControlRef::gotoBookmark() const
{
	lv_.getLyXFunc()->dispatch(LFUN_BOOKMARK_GOTO, "0");
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
	string const name = lv_.buffer()->fileName();
	vector<string>::const_iterator cit =
		find(buffers.begin(), buffers.end(), name);
	if (cit == buffers.end())
		return 0;
	return cit - buffers.begin();
}

string const ControlRef::getBufferName(int num) const
{
	return bufferlist.getFileNames()[num];
}
