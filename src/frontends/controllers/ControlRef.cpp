/**
 * \file ControlRef.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "ControlRef.h"

#include "Buffer.h"
#include "BufferList.h"
#include "FuncRequest.h"

#include "support/filetools.h" // MakeAbsPath, MakeDisplayPath

using lyx::docstring;

using std::vector;
using std::string;

namespace lyx {

using support::makeAbsPath;
using support::makeDisplayPath;

namespace frontend {

ControlRef::ControlRef(Dialog & d)
	: ControlCommand(d, "ref", "ref")
{}


vector<docstring> const ControlRef::getLabelList(string const & name) const
{
	Buffer const & buf = *theBufferList().getBuffer(makeAbsPath(name).absFilename());
	vector<docstring> list;
	buf.getLabelList(list);
	return list;
}


void ControlRef::gotoRef(string const & ref)
{
	kernel().dispatch(FuncRequest(LFUN_BOOKMARK_SAVE, "0"));
	kernel().dispatch(FuncRequest(LFUN_LABEL_GOTO, ref));
}


void ControlRef::gotoBookmark()
{
	kernel().dispatch(FuncRequest(LFUN_BOOKMARK_GOTO, "0"));
}


vector<string> const ControlRef::getBufferList() const
{
	vector<string> buffers = theBufferList().getFileNames();
	for (vector<string>::iterator it = buffers.begin();
	     it != buffers.end(); ++it) {
		*it = lyx::to_utf8(makeDisplayPath(*it));
	}

	return buffers;
}


int ControlRef::getBufferNum() const
{
	vector<string> buffers = theBufferList().getFileNames();
	string const name = kernel().buffer().fileName();
	vector<string>::const_iterator cit =
		std::find(buffers.begin(), buffers.end(), name);
	if (cit == buffers.end())
		return 0;
	return int(cit - buffers.begin());
}

string const ControlRef::getBufferName(int num) const
{
	return theBufferList().getFileNames()[num];
}

} // namespace frontend
} // namespace lyx
