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
#include <map>

using std::endl;
using std::find;
using std::find_if;
using std::getline;
using std::make_pair;
using std::setfill;
using std::setw;
using std::sort;

using std::map;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::vector;

namespace {

double getScalingFactor(Buffer &);

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
	return AddName(bufferpath, filename);
}


/// Store info on a currently executing, forked process.
struct InProgress {
	///
	InProgress() : pid(0) {}
	///
	InProgress(string const & f, vector<StrPair> const & s)
		: pid(0), metrics_file(f), snippets(s)
	{}
	/// Remove any files left lying around and kill the forked process. 
	void stop() const;

	///
	pid_t pid;
	///
	string metrics_file;
	/// Each item in the vector is a pair<snippet, image file name>.
	vector<StrPair> snippets;
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
	///
	static bool haveConverter();
	/// We don't own this
	static Converter const * pconverter_;

	/// Called by the Forkedcall process that generated the bitmap files.
	void finishedGenerating(string const &, pid_t, int);
	///
	void dumpPreamble(ostream &) const;
	///
	void dumpData(ostream &, vector<StrPair> const &) const;
	///
	double fontScalingFactor() const;

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
	vector<string> pending_;

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
	mutable double font_scaling_factor_;
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

void InProgress::stop() const
{
	if (pid)
		ForkedcallsController::get().kill(pid, 0);

	if (!metrics_file.empty())
		lyx::unlink(metrics_file);

	vector<StrPair>::const_iterator vit  = snippets.begin();
	vector<StrPair>::const_iterator vend = snippets.end();
	for (; vit != vend; ++vit) {
		if (!vit->second.empty())
			lyx::unlink(vit->second);
	}
}
 
} // namespace anon


namespace grfx {

bool PreviewLoader::Impl::haveConverter()
{
	if (pconverter_)
		return true;

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
		return true;

	static bool first = true;
	if (first) {
		first = false;
		lyxerr << "PreviewLoader::startLoading()\n"
		       << "No converter from \"lyxpreview\" format has been "
			"defined."
		       << endl;
	}
	
	return false;
}


PreviewLoader::Impl::Impl(PreviewLoader & p, Buffer const & b)
	: parent_(p), buffer_(b), font_scaling_factor_(0.0)
{}


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

	vector<string>::const_iterator vit  = pending_.begin();
	vector<string>::const_iterator vend = pending_.end();
	vit = find(vit, vend, latex_snippet);

	if (vit != vend)
		return InQueue;

	InProgressMap::const_iterator ipit  = in_progress_.begin();
	InProgressMap::const_iterator ipend = in_progress_.end();

	for (; ipit != ipend; ++ipit) {
		vector<StrPair> const & snippets = ipit->second.snippets;
		vector<StrPair>::const_iterator vit  = snippets.begin();
		vector<StrPair>::const_iterator vend = snippets.end();
		vit = find_if(vit, vend, FindFirst(latex_snippet));

		if (vit != vend)
			return Processing;
	}

	return NotFound;
}


void PreviewLoader::Impl::add(string const & latex_snippet)
{
	if (!haveConverter())
		return;

	if (status(latex_snippet) != NotFound)
		return;

	pending_.push_back(latex_snippet);
	sort(pending_.begin(), pending_.end());
}


void PreviewLoader::Impl::remove(string const & latex_snippet)
{
	Cache::iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		cache_.erase(cit);

	vector<string>::iterator vit  = pending_.begin();
	vector<string>::iterator vend = pending_.end();
	vit = find(vit, vend, latex_snippet);

	if (vit != vend)
		pending_.erase(vit, vit+1);

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

	if (!haveConverter())
		return;

	lyxerr[Debug::GRAPHICS] << "PreviewLoader::startLoading()" << endl;

	// As used by the LaTeX file and by the resulting image files
	string const filename_base(unique_filename(buffer_.tmppath));

	// Create an InProgress instance to place in the map of all
	// such processes if it starts correctly.
	vector<StrPair> snippets(pending_.size());
	vector<StrPair>::iterator sit = snippets.begin();
	vector<string>::const_iterator pit  = pending_.begin();
	vector<string>::const_iterator pend = pending_.end();

	int counter = 1; // file numbers start at 1
	for (; pit != pend; ++pit, ++sit, ++counter) {
		ostringstream os;
		os << filename_base
		   << setfill('0') << setw(3) << counter
		   << "." << pconverter_->to;
		string const file = os.str().c_str();

		*sit = make_pair(*pit, file);
	}

	string const metrics_file = filename_base + ".metrics";
	InProgress inprogress(metrics_file, snippets);

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
	double const scaling_factor = fontScalingFactor();
	lyxerr[Debug::GRAPHICS] << "The font scaling factor is "
				<< scaling_factor << endl;
	ostringstream cs;
	cs << pconverter_->command << " " << latexfile << " "
	   << scaling_factor;

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
		// image loader before the PreviewImage/map is properly
		// constucted.
		// This can lead to all sorts of horribleness if such a
		// function attempts to access the cache's internals.
		string const & file = it->second;
		double af = metrics_file.ascent_fraction(metrics_counter++);
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


double PreviewLoader::Impl::fontScalingFactor() const
{
	static double const lyxrc_preview_scale_factor = 0.9;

	if (font_scaling_factor_ > 0.01)
		return font_scaling_factor_;

	font_scaling_factor_ =  getScalingFactor(const_cast<Buffer &>(buffer_));
	return font_scaling_factor_;
}


} // namespace grfx


namespace {

double getScalingFactor(Buffer & buffer)
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
		if (!prefixIs(str, "\\ExecuteOptions"))
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

} // namespace anon
