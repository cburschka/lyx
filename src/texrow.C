/**
 * \file texrow.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "texrow.h"
#include "debug.h"

#include <algorithm>

using std::find_if;


namespace {

/// function object returning true when row number is found
class same_rownumber {
public:
	same_rownumber(int row) : row_(row) {}
	bool operator()(TexRow::RowList::value_type const & vt) const {
		return vt.rownumber() == row_;
	}

private:
	int row_;
};

} // namespace anon


void TexRow::reset()
{
	rowlist.clear();
	count = 0;
	lastid = -1;
	lastpos = -1;
}


void TexRow::start(int id, int pos)
{
	lastid = id;
	lastpos = pos;
}


void TexRow::newline()
{
	int const id = lastid;
	RowList::value_type tmp(id, lastpos, ++count);
	rowlist.push_back(tmp);
}


bool TexRow::getIdFromRow(int row, int & id, int & pos) const
{
	RowList::const_iterator cit =
		find_if(rowlist.begin(), rowlist.end(),
			same_rownumber(row));

	if (cit != rowlist.end()) {
		id = cit->id();
		pos = cit->pos();
		return true;
	}
	id = -1;
	pos = 0;
	return false;
}


TexRow & TexRow::operator+=(TexRow const & tr)
{
	rowlist.insert(rowlist.end(), tr.rowlist.begin(), tr.rowlist.end());
	return *this;
}
