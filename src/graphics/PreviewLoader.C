/*
 *  \file PreviewLoader.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "PreviewLoader.h"
#include "PreviewImage.h"

#include "buffer.h"
#include "bufferparams.h"
#include "converter.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtextclasslist.h"
#include "LColor.h"

#include "insets/inset.h"

#include "frontends/lyx_gui.h" // hexname

#include "support/filetools.h"
#include "support/forkedcall.h"
#include "support/forkedcontr.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

#include <fstream>
#include <iomanip>
#include <list>
#include <map>
#include <utility>
#include <vector>

using std::endl;
using std::find;
using std::fill;
using std::find_if;
using std::getline;
using std::make_pair;
using std::setfill;
using std::setw;

using std::list;
using std::map;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::vector;

namespace {

typedef pair<string, string> StrPair;

typedef list<string> PendingStore;

typedef vector<StrPair> InProgressStore;


double setFontScalingFactor(Buffer &);

string const unique_filename(string const bufferpath);

Converter const * setConverter();

void setAscentFractions(vector<double> & ascent_fractions,
			string const & metrics_file);

struct FindFirst {
	FindFirst(string const & comp) : comp_(comp) {}
	bool operator()(StrPair const & sp)
	{
		return sp.first < comp_;
	}
private:
	string const comp_;
};


/// Store info on a currently executing, forked process.
struct InProgress {
	///
	InProgress() : pid(0) {}
	///
	InProgress(string const & filename_base,
		   PendingStore const & pending,
		   string const & to_format);
	/// Remove any files left lying around and kill the forked process. 
	void stop() const;

	///
	pid_t pid;
	///
	string metrics_file;
	/// Each item in the vector is a pair<snippet, image file name>.
	InProgressStore snippets;
};


} // namespace anon


namespace grfx {

struct PreviewLoader::Impl : public boost::signals::trackable {
	///
	Impl(PreviewLoader & p, Buffer const & b);
	/// Stop any InProgress items still executing.
	~Impl();
	///
	PreviewImage const * preview(string const & latex_snippet) const;
	///
	PreviewLoader::Status status(string const & latex_snippet) const;
	///
	void add(string const & latex_snippet);
	///
	void remove(string const & latex_snippet);
	///
	void startLoading();

private:
	/// Called by the Forkedcall process that generated the bitmap files.
	void finishedGenerating(string const &, pid_t, int);
	///
	void dumpPreamble(ostream &) const;
	///
	void dumpData(ostream &, InProgressStore const &) const;

	/** cache_ allows easy retrieval of already-generated images
	 *  using the LaTeX snippet as the identifier.
	 */
	typedef boost::shared_ptr<PreviewImage> PreviewImagePtr;
	///
	typedef map<string, PreviewImagePtr> Cache;
	///
	Cache cache_;

	/** pending_ stores the LaTeX snippets in anticipation of them being
	 *  sent to the converter.
	 */
	PendingStore pending_;

	/** in_progress_ stores all forked processes so that we can proceed
	 *  thereafter.
	    The map uses the conversion commands as its identifiers.
	 */
	typedef map<string, InProgress> InProgressMap;
	///
	InProgressMap in_progress_;

	///
	PreviewLoader & parent_;
	///
	Buffer const & buffer_;
	///
	double font_scaling_factor_;

	/// We don't own this
	static Converter const * pconverter_;	
};


Converter const * PreviewLoader::Impl::pconverter_;


// The public interface, defined in PreviewLoader.h
// ================================================
PreviewLoader::PreviewLoader(Buffer const & b)
	: pimpl_(new Impl(*this, b))
{}


PreviewLoader::~PreviewLoader()
{}


PreviewImage const * PreviewLoader::preview(string const & latex_snippet) const
{
	return pimpl_->preview(latex_snippet);
}


PreviewLoader::Status PreviewLoader::status(string const & latex_snippet) const
{
	return pimpl_->status(latex_snippet);
}


void PreviewLoader::add(string const & latex_snippet)
{
	pimpl_->add(latex_snippet);
}


void PreviewLoader::remove(string const & latex_snippet)
{
	pimpl_->remove(latex_snippet);
}


void PreviewLoader::startLoading()
{
	pimpl_->startLoading();
}

} // namespace grfx


// The details of the Impl
// =======================

namespace {

InProgress::InProgress(string const & filename_base,
		       PendingStore const & pending,
		       string const & to_format)
	: pid(0),
	  metrics_file(filename_base + ".metrics"),
	  snippets(pending.size())
{
	InProgressStore::iterator sit = snippets.begin();
	PendingStore::const_iterator pit  = pending.begin();
	PendingStore::const_iterator pend = pending.end();

	int counter = 1; // file numbers start at 1
	for (; pit != pend; ++pit, ++sit, ++counter) {
		ostringstream os;
		os << filename_base
		   << setfill('0') << setw(3) << counter
		   << "." << to_format;
		string const file = os.str().c_str();

		*sit = make_pair(*pit, file);
	}
}


void InProgress::stop() const
{
	if (pid)
		ForkedcallsController::get().kill(pid, 0);

	if (!metrics_file.empty())
		lyx::unlink(metrics_file);

	InProgressStore::const_iterator vit  = snippets.begin();
	InProgressStore::const_iterator vend = snippets.end();
	for (; vit != vend; ++vit) {
		if (!vit->second.empty())
			lyx::unlink(vit->second);
	}
}
 
} // namespace anon


namespace grfx {

PreviewLoader::Impl::Impl(PreviewLoader & p, Buffer const & b)
	: parent_(p), buffer_(b), font_scaling_factor_(0.0)
{
	font_scaling_factor_ = setFontScalingFactor(const_cast<Buffer &>(b));

	lyxerr[Debug::GRAPHICS] << "The font scaling factor is "
				<< font_scaling_factor_ << endl;

	if (!pconverter_)
		pconverter_ = setConverter();
}


PreviewLoader::Impl::~Impl()
{
	InProgressMap::iterator ipit  = in_progress_.begin();
	InProgressMap::iterator ipend = in_progress_.end();

	for (; ipit != ipend; ++ipit) {
		ipit->second.stop();
	}
}


PreviewImage const *
PreviewLoader::Impl::preview(string const & latex_snippet) const
{
	Cache::const_iterator it = cache_.find(latex_snippet);
	return (it == cache_.end()) ? 0 : it->second.get();
}


PreviewLoader::Status
PreviewLoader::Impl::status(string const & latex_snippet) const
{
	Cache::const_iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		return Ready;

	PendingStore::const_iterator pit  = pending_.begin();
	PendingStore::const_iterator pend = pending_.end();
	pit = find(pit, pend, latex_snippet);

	if (pit != pend)
		return InQueue;

	InProgressMap::const_iterator ipit  = in_progress_.begin();
	InProgressMap::const_iterator ipend = in_progress_.end();

	for (; ipit != ipend; ++ipit) {
		InProgressStore const & snippets = ipit->second.snippets;
		InProgressStore::const_iterator sit  = snippets.begin();
		InProgressStore::const_iterator send = snippets.end();
		sit = find_if(sit, send, FindFirst(latex_snippet));

		if (sit != send)
			return Processing;
	}

	return NotFound;
}


void PreviewLoader::Impl::add(string const & latex_snippet)
{
	if (!pconverter_ || status(latex_snippet) != NotFound)
		return;

	pending_.push_back(latex_snippet);
}


void PreviewLoader::Impl::remove(string const & latex_snippet)
{
	Cache::iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		cache_.erase(cit);

	PendingStore::iterator pit  = pending_.begin();
	PendingStore::iterator pend = pending_.end();
	pit = find(pit, pend, latex_snippet);

	if (pit != pend) {
		PendingStore::iterator first = pit++;
		pending_.erase(first, pit);
	}

	InProgressMap::iterator ipit  = in_progress_.begin();
	InProgressMap::iterator ipend = in_progress_.end();

	while (ipit != ipend) {
		InProgressMap::iterator curr = ipit;
		++ipit;

		InProgressStore & snippets = curr->second.snippets;
		InProgressStore::iterator sit  = snippets.begin();
		InProgressStore::iterator send = snippets.end();
		sit = find_if(sit, send, FindFirst(latex_snippet));

		if (sit != send)
			snippets.erase(sit, sit+1);

		if (snippets.empty())
			in_progress_.erase(curr);
	}
}


void PreviewLoader::Impl::startLoading()
{
	if (pending_.empty() || !pconverter_)
		return;

	lyxerr[Debug::GRAPHICS] << "PreviewLoader::startLoading()" << endl;

	// As used by the LaTeX file and by the resulting image files
	string const filename_base(unique_filename(buffer_.tmppath));

	// Create an InProgress instance to place in the map of all
	// such processes if it starts correctly.
	InProgress inprogress(filename_base, pending_, pconverter_->to);

	// clear pending_, so we're ready to start afresh.
	pending_.clear();

	// Output the LaTeX file.
	string const latexfile = filename_base + ".tex";

	ofstream of(latexfile.c_str());
	dumpPreamble(of);
	of << "\n\\begin{document}\n";
	dumpData(of, inprogress.snippets);
	of << "\n\\end{document}\n";
	of.close();

	// The conversion command.
	ostringstream cs;
	cs << pconverter_->command << " " << latexfile << " "
	   << font_scaling_factor_;

	string const command = cs.str().c_str();

	// Initiate the conversion from LaTeX to bitmap images files.
	Forkedcall::SignalTypePtr convert_ptr;
	convert_ptr.reset(new Forkedcall::SignalType);

	convert_ptr->connect(
		boost::bind(&Impl::finishedGenerating, this, _1, _2, _3));

	Forkedcall call;
	int ret = call.startscript(command, convert_ptr);

	if (ret != 0) {
		lyxerr[Debug::GRAPHICS] << "PreviewLoader::startLoading()\n"
					<< "Unable to start process \n"
					<< command << endl;
		return;
	}

	// Store the generation process in a list of all such processes
	inprogress.pid = call.pid();
	in_progress_[command] = inprogress;
}


void PreviewLoader::Impl::finishedGenerating(string const & command,
					     pid_t /* pid */, int retval)
{
	string const status = retval > 0 ? "failed" : "succeeded";
	lyxerr[Debug::GRAPHICS] << "PreviewLoader::finishedInProgress("
				<< retval << "): processing " << status
				<< " for " << command << endl;
	if (retval > 0)
		return;

	InProgressMap::iterator git = in_progress_.find(command);
	if (git == in_progress_.end()) {
		lyxerr << "PreviewLoader::finishedGenerating(): unable to find "
			"data for\n"
		       << command << "!" << endl;
		return;
	}

	// Read the metrics file, if it exists
	vector<double> ascent_fractions(git->second.snippets.size());
	setAscentFractions(ascent_fractions, git->second.metrics_file);

	// Add these newly generated bitmap files to the cache and
	// start loading them into LyX.
	InProgressStore::const_iterator it  = git->second.snippets.begin();
	InProgressStore::const_iterator end = git->second.snippets.end();

	int metrics_counter = 0;
	for (; it != end; ++it, ++metrics_counter) {
		string const & snip = it->first;
		string const & file = it->second;
		double af = ascent_fractions[metrics_counter];

		PreviewImagePtr ptr(new PreviewImage(parent_, snip, file, af));
		cache_[snip] = ptr;

		ptr->startLoading();
	}

	// Remove the item from the list of still-executing processes.
	in_progress_.erase(git);
}


void PreviewLoader::Impl::dumpPreamble(ostream & os) const
{
	// Why on earth is Buffer::makeLaTeXFile a non-const method?
	Buffer & tmp = const_cast<Buffer &>(buffer_);
	// Dump the preamble only.
	tmp.makeLaTeXFile(os, string(), true, false, true);

	// Loop over the insets in the buffer and dump all the math-macros.
	Buffer::inset_iterator it  = buffer_.inset_const_iterator_begin();
	Buffer::inset_iterator end = buffer_.inset_const_iterator_end();

	for (; it != end; ++it) {
		if ((*it)->lyxCode() == Inset::MATHMACRO_CODE) {
			(*it)->latex(&buffer_, os, true, true);
		}
	}

	// Use the preview style file to ensure that each snippet appears on a
	// fresh page.
	os << "\n"
	   << "\\usepackage[active,delayed,dvips,tightpage,showlabels]{preview}\n"
	   << "\n";

	// This piece of PostScript magic ensures that the foreground and
	// background colors are the same as the LyX screen.
	string fg = lyx_gui::hexname(LColor::preview);
	if (fg.empty()) fg = "000000";

	string bg = lyx_gui::hexname(LColor::background);
	if (bg.empty()) bg = "ffffff";

	os << "\\AtBeginDocument{\\AtBeginDvi{%\n"
	   << "\\special{!userdict begin/bop-hook{//bop-hook exec\n"
	   << "<" << fg << bg << ">{255 div}forall setrgbcolor\n"
	   << "clippath fill setrgbcolor}bind def end}}}\n";
}


void PreviewLoader::Impl::dumpData(ostream & os,
				   InProgressStore const & vec) const
{
	if (vec.empty())
		return;

	InProgressStore::const_iterator it  = vec.begin();
	InProgressStore::const_iterator end = vec.end();

	for (; it != end; ++it) {
		os << "\\begin{preview}\n"
		   << it->first
		   << "\n\\end{preview}\n\n";
	}
}

} // namespace grfx


namespace {

string const unique_filename(string const bufferpath)
{
	static int theCounter = 0;
	string const filename = tostr(theCounter++) + "lyxpreview";
	return AddName(bufferpath, filename);
}


Converter const * setConverter()
{
	Converter const * converter = 0;

	string const from = "lyxpreview";

	Formats::FormatList::const_iterator it  = formats.begin();
	Formats::FormatList::const_iterator end = formats.end();

	for (; it != end; ++it) {
		string const to = it->name();
		if (from == to)
			continue;

		Converter const * ptr = converters.getConverter(from, to);
		if (ptr)
			return ptr;
	}

	static bool first = true;
	if (first) {
		first = false;
		lyxerr << "PreviewLoader::startLoading()\n"
		       << "No converter from \"lyxpreview\" format has been "
			"defined."
		       << endl;
	}
	
	return 0;
}


double setFontScalingFactor(Buffer & buffer)
{
	static double const lyxrc_preview_scale_factor = 0.9;
	double scale_factor = 0.01 * lyxrc.dpi * lyxrc.zoom *
		lyxrc_preview_scale_factor;

	// Has the font size been set explicitly?
	string const & fontsize = buffer.params.fontsize;
	lyxerr[Debug::GRAPHICS] << "PreviewLoader::scaleToFitLyXView()\n"
				<< "font size is " << fontsize << endl;

	if (isStrUnsignedInt(fontsize))
		return 10.0 * scale_factor / strToDbl(fontsize);

	// No. We must extract it from the LaTeX class file.
	LyXTextClass const & tclass = textclasslist[buffer.params.textclass];
	string const textclass(tclass.latexname() + ".cls");
	string const classfile(findtexfile(textclass, "cls"));

	lyxerr[Debug::GRAPHICS] << "text class is " << textclass << '\n'
				<< "class file is " << classfile << endl;
	
	ifstream ifs(classfile.c_str());
	if (!ifs.good()) {
		lyxerr[Debug::GRAPHICS] << "Unable to open class file!" << endl;
		return scale_factor;
	}

	string str;
	double scaling = scale_factor;

	while (ifs.good()) {
		getline(ifs, str);
		// To get the default font size, look for a line like
		// "\ExecuteOptions{letterpaper,10pt,oneside,onecolumn,final}"
		if (!prefixIs(frontStrip(str), "\\ExecuteOptions"))
			continue;

		str = split(str, '{');
		int count = 0;
		string tok = token(str, ',', count++);
		while (!isValidLength(tok) && !tok.empty())
			tok = token(str, ',', count++);

		if (!tok.empty()) {
			lyxerr[Debug::GRAPHICS]
				<< "Extracted default font size from "
				"LaTeX class file successfully!" << endl;
			LyXLength fsize(tok);
			scaling *= 10.0 / fsize.value();
			break;
		}
	}

	return scaling;
}


void setAscentFractions(vector<double> & ascent_fractions,
			string const & metrics_file)
{
	// If all else fails, then the images will have equal ascents and
	// descents.
	vector<double>::iterator it  = ascent_fractions.begin();
	vector<double>::iterator end = ascent_fractions.end();
	fill(it, end, 0.5);

	ifstream ifs(metrics_file.c_str());
	if (!ifs.good()) {
		lyxerr[Debug::GRAPHICS] << "setAscentFractions("
					<< metrics_file << ")\n"
					<< "Unable to open file!"
					<< endl;
		return;
	}

	for (; it != end; ++it) {
		string page;
		string page_id;
		int dummy;
		int ascent;
		int descent;

		ifs >> page >> page_id >> dummy >> dummy >> dummy >> dummy
		    >> ascent >> descent >> dummy;

		if (!ifs.good() ||
		    page != "%%Page" ||
		    !isStrUnsignedInt(strip(page_id, ':'))) {
			lyxerr[Debug::GRAPHICS] << "setAscentFractions("
						<< metrics_file << ")\n"
						<< "Error reading file!"
						<< endl;
			break;
		}
		
		*it = ascent / (ascent + descent);
	}
}

} // namespace anon
