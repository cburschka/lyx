/*
 *  \file PreviewLoader.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
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

// A list of alll snippets to be converted to previews
typedef list<string> PendingSnippets;

// Each item in the vector is a pair<snippet, image file name>.
typedef vector<StrPair> BitmapFile;


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
		   PendingSnippets const & pending,
		   string const & to_format);
	/// Remove any files left lying around and kill the forked process.
	void stop() const;

	///
	pid_t pid;
	///
	string metrics_file;
	///
	BitmapFile snippets;
};

typedef map<string, InProgress>  InProgressProcesses;

typedef InProgressProcesses::value_type InProgressProcess;

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

	/// Emit this signal when an image is ready for display.
	boost::signal1<void, PreviewImage const &> imageReady;

private:
	/// Called by the Forkedcall process that generated the bitmap files.
	void finishedGenerating(string const &, pid_t, int);
	///
	void dumpPreamble(ostream &) const;
	///
	void dumpData(ostream &, BitmapFile const &) const;

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
	PendingSnippets pending_;

	/** in_progress_ stores all forked processes so that we can proceed
	 *  thereafter.
	    The map uses the conversion commands as its identifiers.
	 */
	InProgressProcesses in_progress_;

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


void PreviewLoader::add(string const & latex_snippet) const
{
	pimpl_->add(latex_snippet);
}


void PreviewLoader::remove(string const & latex_snippet) const
{
	pimpl_->remove(latex_snippet);
}


void PreviewLoader::startLoading() const
{
	pimpl_->startLoading();
}


boost::signals::connection PreviewLoader::connect(slot_type const & slot) const
{
	return pimpl_->imageReady.connect(slot);
}


void PreviewLoader::emitSignal(PreviewImage const & pimage) const
{
	pimpl_->imageReady(pimage);
}

} // namespace grfx


// The details of the Impl
// =======================

namespace {

struct IncrementedFileName {
	IncrementedFileName(string const & to_format,
			    string const & filename_base)
		: to_format_(to_format), base_(filename_base), counter_(1)
	{}

	StrPair const operator()(string const & snippet)
	{
		ostringstream os;
		os << base_
		   << setfill('0') << setw(3) << counter_++
		   << "." << to_format_;

		string const file = os.str().c_str();

		return make_pair(snippet, file);
	}

private:
	string const & to_format_;
	string const & base_;
	int counter_;
};


InProgress::InProgress(string const & filename_base,
		       PendingSnippets const & pending,
		       string const & to_format)
	: pid(0),
	  metrics_file(filename_base + ".metrics"),
	  snippets(pending.size())
{
	PendingSnippets::const_iterator pit  = pending.begin();
	PendingSnippets::const_iterator pend = pending.end();
	BitmapFile::iterator sit = snippets.begin();

	std::transform(pit, pend, sit,
		       IncrementedFileName(to_format, filename_base));
}


void InProgress::stop() const
{
	if (pid)
		ForkedcallsController::get().kill(pid, 0);

	if (!metrics_file.empty())
		lyx::unlink(metrics_file);

	BitmapFile::const_iterator vit  = snippets.begin();
	BitmapFile::const_iterator vend = snippets.end();
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
	InProgressProcesses::iterator ipit  = in_progress_.begin();
	InProgressProcesses::iterator ipend = in_progress_.end();

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


namespace {

struct FindSnippet {
	FindSnippet(string const & s) : snippet_(s) {}
	bool operator()(InProgressProcess const & process)
	{
		BitmapFile const & snippets = process.second.snippets;
		BitmapFile::const_iterator it  = snippets.begin();
		BitmapFile::const_iterator end = snippets.end();
		it = find_if(it, end, FindFirst(snippet_));
		return it != end;
	}

private:
	string const & snippet_;
};

} // namespace anon

PreviewLoader::Status
PreviewLoader::Impl::status(string const & latex_snippet) const
{
	Cache::const_iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		return Ready;

	PendingSnippets::const_iterator pit  = pending_.begin();
	PendingSnippets::const_iterator pend = pending_.end();

	pit = find(pit, pend, latex_snippet);
	if (pit != pend)
		return InQueue;

	InProgressProcesses::const_iterator ipit  = in_progress_.begin();
	InProgressProcesses::const_iterator ipend = in_progress_.end();

	ipit = find_if(ipit, ipend, FindSnippet(latex_snippet));
	if (ipit != ipend)
		return Processing;

	return NotFound;
}


void PreviewLoader::Impl::add(string const & latex_snippet)
{
	if (!pconverter_ || status(latex_snippet) != NotFound)
		return;

	pending_.push_back(latex_snippet);
}


namespace {

struct EraseSnippet {
	EraseSnippet(string const & s) : snippet_(s) {}
	void operator()(InProgressProcess & process)
	{
		BitmapFile & snippets = process.second.snippets;
		BitmapFile::iterator it  = snippets.begin();
		BitmapFile::iterator end = snippets.end();

		it = find_if(it, end, FindFirst(snippet_));
		if (it != end)
			snippets.erase(it, it+1);
	}

private:
	string const & snippet_;
};

} // namespace anon


void PreviewLoader::Impl::remove(string const & latex_snippet)
{
	Cache::iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		cache_.erase(cit);

	PendingSnippets::iterator pit  = pending_.begin();
	PendingSnippets::iterator pend = pending_.end();

	pending_.erase(std::remove(pit, pend, latex_snippet), pend);

	InProgressProcesses::iterator ipit  = in_progress_.begin();
	InProgressProcesses::iterator ipend = in_progress_.end();

	std::for_each(ipit, ipend, EraseSnippet(latex_snippet));

	for (; ipit != ipend; ++ipit) {
		InProgressProcesses::iterator curr = ipit++;
		if (curr->second.snippets.empty())
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
	of << "\\batchmode\n";
	dumpPreamble(of);
	of << "\n\\begin{document}\n";
	dumpData(of, inprogress.snippets);
	of << "\n\\end{document}\n";
	of.close();

	// The conversion command.
	ostringstream cs;
	cs << pconverter_->command << " " << latexfile << " "
	   << font_scaling_factor_;

	string const command = LibScriptSearch(cs.str().c_str());

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

	// Paranoia check!
	InProgressProcesses::iterator git = in_progress_.find(command);
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
	BitmapFile::const_iterator it  = git->second.snippets.begin();
	BitmapFile::const_iterator end = git->second.snippets.end();

	std::list<PreviewImagePtr> newimages;

	int metrics_counter = 0;
	for (; it != end; ++it, ++metrics_counter) {
		string const & snip = it->first;
		string const & file = it->second;
		double af = ascent_fractions[metrics_counter];

		PreviewImagePtr ptr(new PreviewImage(parent_, snip, file, af));
		cache_[snip] = ptr;

		newimages.push_back(ptr);
	}

	// Remove the item from the list of still-executing processes.
	in_progress_.erase(git);

	// Tell the outside world
	std::list<PreviewImagePtr>::const_iterator nit  = newimages.begin();
	std::list<PreviewImagePtr>::const_iterator nend = newimages.end();
	for (; nit != nend; ++nit) {
		imageReady(*nit->get());
	}
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

	// All equation lables appear as "(#)" + preview.sty's rendering of
	// the label name
	if (lyxrc.preview_hashed_labels)
		os << "\\renewcommand{\\theequation}{\\#}\n";

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
				   BitmapFile const & vec) const
{
	if (vec.empty())
		return;

	BitmapFile::const_iterator it  = vec.begin();
	BitmapFile::const_iterator end = vec.end();

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
	double scale_factor = 0.01 * lyxrc.dpi * lyxrc.zoom *
		lyxrc.preview_scale_factor;

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

		// str contains just the options of \ExecuteOptions
		string const tmp = split(str, '{');
		split(tmp, str, '}');

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
		double ascent;
		double descent;

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

		if (ascent + descent != 0)
			*it = ascent / (ascent + descent);
	}
}

} // namespace anon
