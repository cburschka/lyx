/**
 * \file output_plaintext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output_plaintext.h"

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "gettext.h"
#include "output.h"
#include "outputparams.h"
#include "paragraph.h"
#include "ParagraphList_fwd.h"
#include "ParagraphParameters.h"

#include "support/gzstream.h"
#include "support/lstrings.h"

#ifdef HAVE_LOCALE
#endif

using lyx::support::ascii_lowercase;
using lyx::support::compare_ascii_no_case;
using lyx::support::compare_no_case;
using lyx::support::contains;

using lyx::pos_type;
using std::endl;
using std::ostream;
using std::ofstream;
using std::pair;
using std::string;


void writeFileAscii(Buffer const & buf,
		    string const & fname,
		    OutputParams const & runparams)
{
	ofstream ofs;
	if (!::openFileWrite(ofs, fname))
		return;
	writeFileAscii(buf, ofs, runparams);
}


void writeFileAscii(Buffer const & buf, ostream & os,
	OutputParams const & runparams)
{
	Buffer & tmp = const_cast<Buffer &>(buf);
	ParagraphList par = const_cast<ParagraphList&>(tmp.paragraphs());
	ParagraphList::iterator beg = par.begin();
	ParagraphList::iterator end = par.end();
	ParagraphList::iterator it = beg;
	for (; it != end; ++it) {
		asciiParagraph(buf, *it, os, runparams, it == beg);
	}
	os << "\n";
}


namespace {

pair<int, string> const addDepth(int depth, int ldepth)
{
	int d = depth * 2;
	if (ldepth > depth)
		d += (ldepth - depth) * 2;
	return make_pair(d, string(d, ' '));
}

}


void asciiParagraph(Buffer const & buf,
		    Paragraph const & par,
		    ostream & os,
		    OutputParams const & runparams,
		    bool noparbreak)
{
	int ltype = 0;
	Paragraph::depth_type ltype_depth = 0;
	bool ref_printed = false;
	Paragraph::depth_type depth = par.params().depth();

	// First write the layout
	string const & tmp = par.layout()->name();
	if (compare_no_case(tmp, "itemize") == 0) {
		ltype = 1;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "enumerate") == 0) {
		ltype = 2;
		ltype_depth = depth + 1;
	} else if (contains(ascii_lowercase(tmp), "ection")) {
		ltype = 3;
		ltype_depth = depth + 1;
	} else if (contains(ascii_lowercase(tmp), "aragraph")) {
		ltype = 4;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "description") == 0) {
		ltype = 5;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "abstract") == 0) {
		ltype = 6;
		ltype_depth = 0;
	} else if (compare_ascii_no_case(tmp, "bibliography") == 0) {
		ltype = 7;
		ltype_depth = 0;
	} else {
		ltype = 0;
		ltype_depth = 0;
	}

	/* maybe some vertical spaces */

	/* the labelwidthstring used in lists */

	/* some lines? */

	/* some pagebreaks? */

	/* noindent ? */

	/* what about the alignment */

	// runparams.linelen <= 0 is special and means we don't have paragraph breaks

	string::size_type currlinelen = 0;

	if (!noparbreak) {
		if (runparams.linelen > 0)
			os << "\n\n";

		os << string(depth * 2, ' ');
		currlinelen += depth * 2;

		//--
		// we should probably change to the paragraph language in the
		// gettext here (if possible) so that strings are output in
		// the correct language! (20012712 Jug)
		//--
		switch (ltype) {
		case 0: // Standard
		case 4: // (Sub)Paragraph
		case 5: // Description
			break;

		case 6: // Abstract
			if (runparams.linelen > 0) {
				os << _("Abstract") << "\n\n";
				currlinelen = 0;
			} else {
				string const abst = _("Abstract: ");
				os << abst;
				currlinelen += abst.length();
			}
			break;

		case 7: // Bibliography
			if (!ref_printed) {
				if (runparams.linelen > 0) {
					os << _("References") << "\n\n";
					currlinelen = 0;
				} else {
					string const refs = _("References: ");
					os << refs;
					currlinelen += refs.length();
				}
				ref_printed = true;
			}
			break;

		default: {
			string const label = par.params().labelString();
			os << label << ' ';
			currlinelen += label.length() + 1;
			break;
		}

		}
	}

	if (!currlinelen) {
		pair<int, string> p = addDepth(depth, ltype_depth);
		os << p.second;
		currlinelen += p.first;
	}

	// this is to change the linebreak to do it by word a bit more
	// intelligent hopefully! (only in the case where we have a
	// max runparams.linelength!) (Jug)

	string word;

	for (pos_type i = 0; i < par.size(); ++i) {
		char c = par.getUChar(buf.params(), i);
		switch (c) {
		case Paragraph::META_INSET: {
			InsetBase const * inset = par.getInset(i);
			if (runparams.linelen > 0) {
				os << word;
				currlinelen += word.length();
				word.erase();
			}
			if (inset->plaintext(buf, os, runparams)) {
				// to be sure it breaks paragraph
				currlinelen += runparams.linelen;
			}
			break;
		}

		case ' ':
			if (runparams.linelen > 0 &&
			    currlinelen + word.length() > runparams.linelen - 10) {
				os << "\n";
				pair<int, string> p = addDepth(depth, ltype_depth);
				os << p.second;
				currlinelen = p.first;
			}
			os << word << ' ';
			currlinelen += word.length() + 1;
			word.erase();
			break;

		case '\0':
			lyxerr[Debug::INFO] <<
				"writeAsciiFile: NULL char in structure." << endl;
			break;

		default:
			word += c;
			if (runparams.linelen > 0 &&
			    currlinelen + word.length() > runparams.linelen)
			{
				os << "\n";
				pair<int, string> p = addDepth(depth, ltype_depth);
				os << p.second;
				currlinelen = p.first;
			}
			break;
		}
	}
	os << word;
}
