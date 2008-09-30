/**
 * \file Changes.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 *
 * Record changes in a paragraph.
 */

#include <config.h>

#include "Changes.h"
#include "Author.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "LaTeXFeatures.h"
#include "Paragraph.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <ostream>

using namespace std;

namespace lyx {

/*
 * Class Change has a changetime field that specifies the exact time at which
 * a specific change was made. The change time is used as a guidance for the
 * user while editing his document. Presently, it is not considered for LaTeX
 * export.
 * When merging two adjacent changes, the changetime is not considered,
 * only the equality of the change type and author is checked (in method
 * isSimilarTo(...)). If two changes are in fact merged (in method merge()),
 * the later change time is preserved.
 */

bool Change::isSimilarTo(Change const & change) const
{
	if (type != change.type)
		return false;

	if (type == Change::UNCHANGED)
		return true;

	return author == change.author;
}


ColorCode Change::color() const
{
	ColorCode color = Color_none;
	switch (author % 5) {
		case 0:
			color = Color_changedtextauthor1;
			break;
		case 1:
			color = Color_changedtextauthor2;
			break;
		case 2:
			color = Color_changedtextauthor3;
			break;
		case 3:
			color = Color_changedtextauthor4;
			break;
		case 4:
			color = Color_changedtextauthor5;
			break;
	}
	return color;
}


bool operator==(Change const & l, Change const & r)
{
	if (l.type != r.type)
		return false;

	// two changes of type UNCHANGED are always equal
	if (l.type == Change::UNCHANGED)
		return true;

	return l.author == r.author && l.changetime == r.changetime;
}


bool operator!=(Change const & l, Change const & r)
{
	return !(l == r);
}


bool operator==(Changes::Range const & r1, Changes::Range const & r2)
{
	return r1.start == r2.start && r1.end == r2.end;
}


bool operator!=(Changes::Range const & r1, Changes::Range const & r2)
{
	return !(r1 == r2);
}


bool Changes::Range::intersects(Range const & r) const
{
	return r.start < end && r.end > start; // end itself is not in the range!
}


void Changes::set(Change const & change, pos_type const pos)
{
	set(change, pos, pos + 1);
}


void Changes::set(Change const & change, pos_type const start, pos_type const end)
{
	if (change.type != Change::UNCHANGED) {
		LYXERR(Debug::CHANGES, "setting change (type: " << change.type
			<< ", author: " << change.author 
			<< ", time: " << long(change.changetime)
			<< ") in range (" << start << ", " << end << ")");
	}

	Range const newRange(start, end);

	ChangeTable::iterator it = table_.begin();

	for (; it != table_.end(); ) {
		// current change starts like or follows new change
		if (it->range.start >= start) {
			break;
		}

		// new change intersects with existing change
		if (it->range.end > start) {
			pos_type oldEnd = it->range.end;
			it->range.end = start;

			LYXERR(Debug::CHANGES, "  cutting tail of type " << it->change.type
				<< " resulting in range (" << it->range.start << ", "
				<< it->range.end << ")");

			++it;
			if (oldEnd >= end) {
				LYXERR(Debug::CHANGES, "  inserting tail in range ("
					<< end << ", " << oldEnd << ")");
				it = table_.insert(it, ChangeRange((it-1)->change, Range(end, oldEnd)));
			}
			continue;
		}

		++it;
	}

	if (change.type != Change::UNCHANGED) {
		LYXERR(Debug::CHANGES, "  inserting change");
		it = table_.insert(it, ChangeRange(change, Range(start, end)));
		++it;
	}

	for (; it != table_.end(); ) {
		// new change 'contains' existing change
		if (newRange.contains(it->range)) {
			LYXERR(Debug::CHANGES, "  removing subrange ("
				<< it->range.start << ", " << it->range.end << ")");
			it = table_.erase(it);
			continue;
		}

		// new change precedes existing change
		if (it->range.start >= end)
			break;

		// new change intersects with existing change
		it->range.start = end;
		LYXERR(Debug::CHANGES, "  cutting head of type "
			<< it->change.type << " resulting in range ("
			<< end << ", " << it->range.end << ")");
		break; // no need for another iteration
	}

	merge();
}


void Changes::erase(pos_type const pos)
{
	LYXERR(Debug::CHANGES, "Erasing change at position " << pos);

	ChangeTable::iterator it = table_.begin();
	ChangeTable::iterator end = table_.end();

	for (; it != end; ++it) {
		// range (pos,pos+x) becomes (pos,pos+x-1)
		if (it->range.start > pos)
			--(it->range.start);
		// range (pos-x,pos) stays (pos-x,pos)
		if (it->range.end > pos)
			--(it->range.end);
	}

	merge();
}


void Changes::insert(Change const & change, lyx::pos_type pos)
{
	if (change.type != Change::UNCHANGED) {
		LYXERR(Debug::CHANGES, "Inserting change of type " << change.type
			<< " at position " << pos);
	}

	ChangeTable::iterator it = table_.begin();
	ChangeTable::iterator end = table_.end();

	for (; it != end; ++it) {
		// range (pos,pos+x) becomes (pos+1,pos+x+1)
		if (it->range.start >= pos)
			++(it->range.start);

		// range (pos-x,pos) stays as it is
		if (it->range.end > pos)
			++(it->range.end);
	}

	set(change, pos, pos + 1); // set will call merge
}


Change const & Changes::lookup(pos_type const pos) const
{
	static Change const noChange = Change(Change::UNCHANGED);

	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const end = table_.end();

	for (; it != end; ++it) {
		if (it->range.contains(pos))
			return it->change;
	}

	return noChange;
}


bool Changes::isChanged(pos_type const start, pos_type const end) const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const itend = table_.end();

	for (; it != itend; ++it) {
		if (it->range.intersects(Range(start, end))) {
			LYXERR(Debug::CHANGES, "found intersection of range ("
				<< start << ", " << end << ") with ("
				<< it->range.start << ", " << it->range.end
				<< ") of type " << it->change.type);
			return true;
		}
	}
	return false;
}


void Changes::merge()
{
	ChangeTable::iterator it = table_.begin();

	while (it != table_.end()) {
		LYXERR(Debug::CHANGES, "found change of type " << it->change.type
			<< " and range (" << it->range.start << ", " << it->range.end
			<< ")");

		if (it->range.start == it->range.end) {
			LYXERR(Debug::CHANGES, "removing empty range for pos "
				<< it->range.start);

			table_.erase(it);
			// start again
			it = table_.begin();
			continue;
		}

		if (it + 1 == table_.end())
			break;

		if (it->change.isSimilarTo((it + 1)->change)
		    && it->range.end == (it + 1)->range.start) {
			LYXERR(Debug::CHANGES, "merging ranges (" << it->range.start << ", "
				<< it->range.end << ") and (" << (it + 1)->range.start << ", "
				<< (it + 1)->range.end << ")");

			(it + 1)->range.start = it->range.start;
			(it + 1)->change.changetime = max(it->change.changetime,
							  (it + 1)->change.changetime);
			table_.erase(it);
			// start again
			it = table_.begin();
			continue;
		}

		++it;
	}
}


int Changes::latexMarkChange(odocstream & os, BufferParams const & bparams,
			     Change const & oldChange, Change const & change)
{
	if (!bparams.outputChanges || oldChange == change)
		return 0;

	int column = 0;

	if (oldChange.type != Change::UNCHANGED) {
		os << '}'; // close \lyxadded or \lyxdeleted
		column++;
	}

	docstring chgTime;
	chgTime += ctime(&change.changetime);
	chgTime.erase(chgTime.end() - 1); // remove trailing '\n'

	if (change.type == Change::DELETED) {
		docstring str = "\\lyxdeleted{" +
			bparams.authors().get(change.author).name() + "}{" +
			chgTime + "}{";
		os << str;
		column += str.size();
	} else if (change.type == Change::INSERTED) {
		docstring str = "\\lyxadded{" +
			bparams.authors().get(change.author).name() + "}{" +
			chgTime + "}{";
		os << str;
		column += str.size();
	}

	return column;
}


void Changes::lyxMarkChange(ostream & os, int & column,
			    Change const & old, Change const & change)
{
	if (old == change)
		return;

	column = 0;

	switch (change.type) {
		case Change::UNCHANGED:
			os << "\n\\change_unchanged\n";
			break;

		case Change::DELETED: {
			os << "\n\\change_deleted " << change.author
				<< " " << change.changetime << "\n";
			break;
		}

		case Change::INSERTED: {
			os << "\n\\change_inserted " << change.author
				<< " " << change.changetime << "\n";
			break;
		}
	}
}


void Changes::checkAuthors(AuthorList const & authorList)
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator endit = table_.end();
	for ( ; it != endit ; ++it) 
		if (it->change.type != Change::UNCHANGED)
			authorList.get(it->change.author).setUsed(true);
}


void Changes::addToToc(DocIterator const & cdit, Buffer const & buffer) const
{
	if (table_.empty())
		return;

	Toc & change_list = buffer.tocBackend().toc("change");
	AuthorList const & author_list = buffer.params().authors();
	DocIterator dit = cdit;

	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const itend = table_.end();
	for (; it != itend; ++it) {
		docstring str;
		switch (it->change.type) {
		case Change::UNCHANGED:
			continue;
		case Change::DELETED:
			// 0x2702 is a scissors symbol in the Dingbats unicode group.
			str.push_back(0x2702);
			break;
		case Change::INSERTED:
			// 0x2702 is the hand writting symbol in the Dingbats unicode group.
			str.push_back(0x270d);
			break;
		}
		dit.pos() = it->range.start;
		str += " " + dit.paragraph().asString(it->range.start, it->range.end);
		docstring const & author = author_list.get(it->change.author).name();
		Toc::iterator it = change_list.item(0, author);
		if (it == change_list.end()) {
			change_list.push_back(TocItem(dit, 0, author));
			change_list.push_back(TocItem(dit, 1, str));
			continue;
		}
		for (++it; it != change_list.end(); ++it) {
			if (it->depth() == 0 && it->str() != author)
				break;
		}
		change_list.insert(it, TocItem(dit, 1, str));
	}
}

} // namespace lyx
