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
#include "PreviewMetrics.h"

#include "buffer.h"
#include "bufferparams.h"
#include "converter.h"
#include "debug.h"
#include "lyxrc.h"
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
#include <map>

using std::endl;
using std::find_if;
using std::setfill;
using std::setw;
using std::sort;

using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::vector;

namespace {

typedef pair<string, string> StrPair;

struct CompSecond {
	bool operator()(StrPair const & lhs, StrPair const & rhs)
	{
		return lhs.second < rhs.second;
	}
};

struct FindFirst {
	FindFirst(string const & comp) : comp_(comp) {}
	bool operator()(StrPair const & sp)
	{
		return sp.first < comp_;
	}
private:
	string const comp_;
};


string const unique_filename(string const bufferpath)
{
	static int theCounter = 0;
	string const filename = tostr(theCounter++) + "lyxpreview";
	return AddName(bufferpath, filename);ostringstream os;
}

} // namespace anon


namespace grfx {

struct PreviewLoader::Impl : public boost::signals::trackable {
	///
	Impl(PreviewLoader & p, Buffer const & b);
	///
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

	///
	typedef pair<string, string> StrPair;
	///
	typedef map<string, string> PendingMap;

private:
	/// Called by the Forkedcall process that generated the bitmap files.
	void finishedGenerating(string const &, pid_t, int);
	///
	void dumpPreamble(ostream &) const;
	///
	void dumpData(ostream &, vector<StrPair> const &) const;

	///
	static void setConverter();
	/// We don't own this
	static Converter const * pconverter_;

	/** cache_ allows easy retrieval of already-generated images
	 *  using the LaTeX snippet as the identifier.
	 */
	typedef boost::shared_ptr<PreviewImage> PreviewImagePtr;
	///
	typedef map<string, PreviewImagePtr> Cache;
	///
	Cache cache_;

	/** pending_ stores the LaTeX snippet and the name of the generated
	 *  bitmap image file in anticipation of them being sent to the
	 *  converter.
	 */
	PendingMap pending_;

	/// Store info on a currently executing, forked process.
	struct InProgress {
		///
		InProgress() {}
		///
		InProgress(string const & f, PendingMap const & m)
			: pid(0), metrics_file(f), snippets(m.begin(), m.end())
		{
			sort(snippets.begin(), snippets.end(), CompSecond());
		}

		///
		pid_t pid;
		///
		string metrics_file;

		/** Store the info in the PendingMap as a vector.
		    Ensures that the data is output in the order
		    file001, file002 etc, as we expect, which is /not/ what
		    happens when we iterate through the map.
		 */
		vector<StrPair> snippets;
	};
	
	/// Store all forked processes so that we can proceed thereafter.
	typedef map<string, InProgress> InProgressMap;
	///
	InProgressMap in_progress_;

	///
	string filename_base_;
	///
	PreviewLoader & parent_;
	///
	Buffer const & buffer_;
};

Converter const * PreviewLoader::Impl::pconverter_;


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


void PreviewLoader::Impl::setConverter()
{
	if (pconverter_)
		return;

	string const from = "lyxpreview";

	Formats::FormatList::const_iterator it  = formats.begin();
	Formats::FormatList::const_iterator end = formats.end();

	for (; it != end; ++it) {
		string const to = it->name();
		if (from == to)
			continue;
		Converter const * ptr = converters.getConverter(from, to);
		if (ptr) {
			pconverter_ = ptr;
			break;
		}
	}

	if (pconverter_)
		return;

	static bool first = true;
	if (!first)
		return;

	first = false;
	lyxerr << "PreviewLoader::startLoading()\n"
	       << "No converter from \"lyxpreview\" format has been defined." 
	       << endl;
}


PreviewLoader::Impl::Impl(PreviewLoader & p, Buffer const & b)
	: filename_base_(unique_filename(b.tmppath)), parent_(p), buffer_(b)
{}


PreviewImage const *
PreviewLoader::Impl::preview(string const & latex_snippet) const
{
	Cache::const_iterator it = cache_.find(latex_snippet);
	return (it == cache_.end()) ? 0 : it->second.get();
}


PreviewLoader::Impl::~Impl()
{
	InProgressMap::const_iterator ipit  = in_progress_.begin();
	InProgressMap::const_iterator ipend = in_progress_.end();

	for (; ipit != ipend; ++ipit) {
		pid_t pid = ipit->second.pid;
		if (pid)
			ForkedcallsController::get().kill(pid, 0);

		lyx::unlink(ipit->second.metrics_file);

		vector<StrPair> const & snippets = ipit->second.snippets;
		vector<StrPair>::const_iterator vit  = snippets.begin();
		vector<StrPair>::const_iterator vend = snippets.end();
		for (; vit != vend; ++vit) {
			lyx::unlink(vit->second);
		}
	}
}


PreviewLoader::Status
PreviewLoader::Impl::status(string const & latex_snippet) const
{
	Cache::const_iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		return PreviewLoader::Ready;

	PendingMap::const_iterator pit = pending_.find(latex_snippet);
	if (pit != pending_.end())
		return PreviewLoader::InQueue;

	InProgressMap::const_iterator ipit  = in_progress_.begin();
	InProgressMap::const_iterator ipend = in_progress_.end();

	for (; ipit != ipend; ++ipit) {
		vector<StrPair> const & snippets = ipit->second.snippets;
		vector<StrPair>::const_iterator vit  = snippets.begin();
		vector<StrPair>::const_iterator vend = snippets.end();
		vit = find_if(vit, vend, FindFirst(latex_snippet));
		
		if (vit != vend)
			return PreviewLoader::Processing;
	}

	return PreviewLoader::NotFound;
}


void PreviewLoader::Impl::add(string const & latex_snippet)
{
	if (!pconverter_) {
		setConverter();
		if (!pconverter_)
			return;
	}

	Cache::const_iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		return;

	PendingMap::const_iterator pit = pending_.find(latex_snippet);
	if (pit != pending_.end())
		return;

	int const snippet_counter = int(pending_.size()) + 1;
	ostringstream os;
	os << filename_base_
	   << setfill('0') << setw(3) << snippet_counter
	   << "." << pconverter_->to;
	string const image_filename = os.str().c_str();

	pending_[latex_snippet] = image_filename;
}


void PreviewLoader::Impl::remove(string const & latex_snippet)
{
	Cache::iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		cache_.erase(cit);

	PendingMap::iterator pit = pending_.find(latex_snippet);
	if (pit != pending_.end())
		pending_.erase(pit);

	InProgressMap::iterator ipit  = in_progress_.begin();
	InProgressMap::iterator ipend = in_progress_.end();

	while (ipit != ipend) {
		InProgressMap::iterator curr = ipit;
		++ipit;

		vector<StrPair> & snippets = curr->second.snippets;
		vector<StrPair>::iterator vit  = snippets.begin();
		vector<StrPair>::iterator vend = snippets.end();
		vit = find_if(vit, vend, FindFirst(latex_snippet));
		
		if (vit != vend)
			snippets.erase(vit, vit+1);

		if (snippets.empty())
			in_progress_.erase(curr);
	}
}


void PreviewLoader::Impl::startLoading()
{
	if (pending_.empty())
		return;

	if (!pconverter_) {
		setConverter();
		if (!pconverter_)
			return;
	}

	lyxerr[Debug::GRAPHICS] << "PreviewLoader::startLoading()" << endl;

	// Create an InProgress instance to place in the map of all
	// such processes if it starts correctly.
	string const metrics_file = filename_base_ + ".metrics";
	InProgress inprogress(metrics_file, pending_);

	// Output the LaTeX file.
	string const latexfile = filename_base_ + ".tex";

	ofstream of(latexfile.c_str());
	dumpPreamble(of);
	of << "\n\\begin{document}\n";
	dumpData(of, inprogress.snippets);
	of << "\n\\end{document}\n";
	of.close();

	// Reset the filename and clear pending_, so we're ready to
	// start afresh.
	pending_.clear();
	filename_base_ = unique_filename(buffer_.tmppath);

	// The conversion command.
	ostringstream cs;
	cs << pconverter_->command << " " << latexfile << " "
	   << tostr(0.01 * lyxrc.dpi * lyxrc.zoom);

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

	// Reset the pid to 0 as the process has finished.
	git->second.pid = 0;

	// Read the metrics file, if it exists
	PreviewMetrics metrics_file(git->second.metrics_file);
	
	// Add these newly generated bitmap files to the cache and
	// start loading them into LyX.
	vector<StrPair>::const_iterator it  = git->second.snippets.begin();
	vector<StrPair>::const_iterator end = git->second.snippets.end();

	int metrics_counter = 0;
	for (; it != end; ++it) {
		string const & snip = it->first;

		// Paranoia check
		Cache::const_iterator chk = cache_.find(snip);
		if (chk != cache_.end())
			continue;

		// Mental note (Angus, 4 July 2002, having just found out the
		// hard way :-().
		// We /must/ first add to the cache and then start the
		// image loading process.
		// If not, then outside functions can be called before by the
		// image loader before the PreviewImage is properly constucted.
		// This can lead to all sorts of horribleness if such a
		// function attempts to access its internals.
		string const & file = it->second;
		double af = metrics_file.ascent_fraction(metrics_counter++);
		PreviewImagePtr ptr(new PreviewImage(parent_, snip, file, af));

		cache_[snip] = ptr;

		ptr->startLoading();
	}

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
	   << "\\usepackage[active,dvips,tightpage]{preview}\n"
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
				   vector<StrPair> const & vec) const
{
	if (vec.empty())
		return;

	vector<StrPair>::const_iterator it  = vec.begin();
	vector<StrPair>::const_iterator end = vec.end();

	for (; it != end; ++it) {
		os << "\\begin{preview}\n"
		   << it->first 
		   << "\n\\end{preview}\n\n";
	}
}

} // namespace grfx
