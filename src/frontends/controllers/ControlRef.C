/**
 * \file ControlRef.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlRef.h"
#include "lyxfunc.h"
#include "buffer.h"
#include "bufferlist.h"
#include "funcrequest.h"

#include "support/filetools.h" // MakeAbsPath, MakeDisplayPath

using namespace lyx::support;

using std::vector;

extern BufferList bufferlist;


ControlRef::ControlRef(Dialog & d)
	: ControlCommand(d, "ref")
{}


vector<string> const ControlRef::getLabelList(string const & name) const
{
	Buffer const * buf = bufferlist.getBuffer(MakeAbsPath(name));
	if (!buf)
		buf = kernel().buffer();
	vector<string> list;
	buf->getLabelList(list);
	return list;
}


void ControlRef::gotoRef(string const & ref)
{
	kernel().dispatch(FuncRequest(LFUN_BOOKMARK_SAVE, "0"), false);
	kernel().dispatch(FuncRequest(LFUN_REF_GOTO, ref));
}


void ControlRef::gotoBookmark()
{
	kernel().dispatch(FuncRequest(LFUN_BOOKMARK_GOTO, "0"), false);
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
	string const name = kernel().buffer()->fileName();
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
