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
#include "Encoding.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/mutex.h"

#include "frontends/alert.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <ostream>

using namespace std;

namespace lyx {

using frontend::Painter;
using frontend::FontMetrics;

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


Color Change::color() const
{
	Color color = Color_none;
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

	if (deleted())
		color.mergeColor = Color_deletedtextmodifier;

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
		if (!isChanged())
			is_update_required_ = true;
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


bool Changes::isDeleted(pos_type start, pos_type end) const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const itend = table_.end();

	for (; it != itend; ++it) {
		if (it->range.contains(Range(start, end))) {
			LYXERR(Debug::CHANGES, "range ("
				<< start << ", " << end << ") fully contains ("
				<< it->range.start << ", " << it->range.end
				<< ") of type " << it->change.type);
			return it->change.type == Change::DELETED;
		}
	}
	return false;
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


bool Changes::isChanged() const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const itend = table_.end();
	for (; it != itend; ++it) {
		if (it->change.changed())
			return true;
	}
	return false;
}


void Changes::merge()
{
	bool merged = false;
	ChangeTable::iterator it = table_.begin();

	while (it != table_.end()) {
		LYXERR(Debug::CHANGES, "found change of type " << it->change.type
			<< " and range (" << it->range.start << ", " << it->range.end
			<< ")");

		if (it->range.start == it->range.end) {
			LYXERR(Debug::CHANGES, "removing empty range for pos "
				<< it->range.start);

			table_.erase(it);
			merged = true;
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
			merged = true;
			// start again
			it = table_.begin();
			continue;
		}

		++it;
	}
	if (merged && !isChanged())
		is_update_required_ = true;
}


namespace {

docstring getLaTeXMarkup(docstring const & macro, docstring const & author,
			 docstring const & chgTime,
			 OutputParams const & runparams)
{
	if (macro.empty())
		return docstring();

	docstring uncodable_author;
	odocstringstream ods;

	ods << macro;
	// convert utf8 author name to something representable
	// in the current encoding
	pair<docstring, docstring> author_latexed =
		runparams.encoding->latexString(author, runparams.dryrun);
	if (!author_latexed.second.empty()) {
		LYXERR0("Omitting uncodable characters '"
			<< author_latexed.second
			<< "' in change author name!");
		uncodable_author = author;
	}
	ods << author_latexed.first << "}{" << chgTime << "}{";

	// warn user (once) if we found uncodable glyphs.
	if (!uncodable_author.empty()) {
		static std::set<docstring> warned_authors;
		static Mutex warned_mutex;
		Mutex::Locker locker(&warned_mutex);
		if (warned_authors.find(uncodable_author) == warned_authors.end()) {
			frontend::Alert::warning(_("Uncodable character in author name"),
				support::bformat(_("The author name '%1$s',\n"
				  "used for change tracking, contains the following glyphs that\n"
				  "cannot be represented in the current encoding: %2$s.\n"
				  "These glyphs will be omitted in the exported LaTeX file.\n\n"
				  "Choose an appropriate document encoding (such as utf8)\n"
				  "or change the spelling of the author name."),
				uncodable_author, author_latexed.second));
			warned_authors.insert(uncodable_author);
		}
	}

	return ods.str();
}

} //namespace anon


int Changes::latexMarkChange(otexstream & os, BufferParams const & bparams,
			     Change const & oldChange, Change const & change,
			     OutputParams const & runparams)
{
	if (!bparams.output_changes || oldChange == change)
		return 0;

	int column = 0;

	if (oldChange.type != Change::UNCHANGED) {
		// close \lyxadded or \lyxdeleted
		os << '}';
		column++;
		if (oldChange.type == Change::DELETED)
			--runparams.inulemcmd;
	}

	docstring chgTime;
	chgTime += asctime(gmtime(&change.changetime));
	// remove trailing '\n'
	chgTime.erase(chgTime.end() - 1);

	docstring macro_beg;
	if (change.type == Change::DELETED) {
		macro_beg = from_ascii("\\lyxdeleted{");
		++runparams.inulemcmd;
	}
	else if (change.type == Change::INSERTED)
		macro_beg = from_ascii("\\lyxadded{");
	
	docstring str = getLaTeXMarkup(macro_beg,
				       bparams.authors().get(change.author).name(),
				       chgTime, runparams);
	
	os << str;
	column += str.size();

	return column;
}


void Changes::lyxMarkChange(ostream & os, BufferParams const & bparams, int & column,
			    Change const & old, Change const & change)
{
	if (old == change)
		return;

	column = 0;

	int const buffer_id = bparams.authors().get(change.author).bufferId();

	switch (change.type) {
		case Change::UNCHANGED:
			os << "\n\\change_unchanged\n";
			break;

		case Change::DELETED:
			os << "\n\\change_deleted " << buffer_id
				<< " " << change.changetime << "\n";
			break;

		case Change::INSERTED:
			os << "\n\\change_inserted " << buffer_id
				<< " " << change.changetime << "\n";
			break;
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


void Changes::addToToc(DocIterator const & cdit, Buffer const & buffer,
        bool output_active) const
{
	if (table_.empty())
		return;

	shared_ptr<Toc> change_list = buffer.tocBackend().toc("change");
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
			// ✂ U+2702 BLACK SCISSORS
			str.push_back(0x2702);
			break;
		case Change::INSERTED:
			// ✍ U+270D WRITING HAND
			str.push_back(0x270d);
			break;
		}
		dit.pos() = it->range.start;
		Paragraph const & par = dit.paragraph();
		str += " " + par.asString(it->range.start, min(par.size(), it->range.end));
		if (it->range.end > par.size())
			// ¶ U+00B6 PILCROW SIGN
			str.push_back(0xb6);
		docstring const & author = author_list.get(it->change.author).name();
		Toc::iterator it = TocBackend::findItem(*change_list, 0, author);
		if (it == change_list->end()) {
			change_list->push_back(TocItem(dit, 0, author, true));
			change_list->push_back(TocItem(dit, 1, str, output_active,
				support::wrapParas(str, 4)));
			continue;
		}
		for (++it; it != change_list->end(); ++it) {
			if (it->depth() == 0 && it->str() != author)
				break;
		}
		change_list->insert(it, TocItem(dit, 1, str, output_active,
			support::wrapParas(str, 4)));
	}
}


void Changes::updateBuffer(Buffer const & buf)
{
	is_update_required_ = false;
	if (!buf.areChangesPresent() && isChanged())
		buf.setChangesPresent(true);
}


void Change::paintCue(PainterInfo & pi, double const x1, double const y,
                      double const x2, FontInfo const & font) const
{
	if (!changed())
		return;
	// Calculate 1/3 height of font
	FontMetrics const & fm = theFontMetrics(font);
	int const y_bar = deleted() ? y - fm.maxAscent() / 3
		: y + 2 * pi.base.solidLineOffset() + pi.base.solidLineThickness();
	pi.pain.line(int(x1), y_bar, int(x2), y_bar, color(),
	             Painter::line_solid, pi.base.solidLineThickness());
}


void Change::paintCue(PainterInfo & pi, double const x1, double const y1,
                      double const x2, double const y2) const
{
	/*
	 * y1      /
	 *        /
	 *       /
	 *      /
	 *     /
	 * y2 /_____
	 *    x1  x2
	 */
	double y = 0;
	switch(type) {
	case UNCHANGED:
		return;
	case INSERTED:
		y = y2;
		break;
	case DELETED:
		y = y1;
		break;
	}
	pi.pain.line(x1, y2, x2, y, color(), Painter::line_solid,
	             pi.base.solidLineThickness());
}


} // namespace lyx
