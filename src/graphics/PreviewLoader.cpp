/**
 * \file PreviewLoader.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PreviewLoader.h"
#include "PreviewImage.h"
#include "GraphicsCache.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Converter.h"
#include "debug.h"
#include "Encoding.h"
#include "Format.h"
#include "InsetIterator.h"
#include "Color.h"
#include "LaTeXFeatures.h"
#include "LyXRC.h"
#include "output.h"
#include "OutputParams.h"
#include "TexRow.h"

#include "frontends/Application.h" // hexName

#include "insets/Inset.h"

#include "support/filetools.h"
#include "support/Forkedcall.h"
#include "support/ForkedcallsController.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/convert.h"

#include <boost/bind.hpp>

#include <sstream>
#include <fstream>
#include <iomanip>

using lyx::support::FileName;

using std::endl;
using std::find;
using std::fill;
using std::find_if;
using std::make_pair;

using boost::bind;

using std::ifstream;
using std::list;
using std::map;
using std::ostringstream;
using std::pair;
using std::vector;
using std::string;


namespace {

typedef pair<string, FileName> SnippetPair;

// A list of all snippets to be converted to previews
typedef list<string> PendingSnippets;

// Each item in the vector is a pair<snippet, image file name>.
typedef vector<SnippetPair> BitmapFile;


string const unique_filename(string const & bufferpath)
{
	static int theCounter = 0;
	string const filename = lyx::convert<string>(theCounter++) + "lyxpreview";
	return lyx::support::addName(bufferpath, filename);
}


lyx::Converter const * setConverter()
{
	string const from = "lyxpreview";

	typedef vector<string> FmtList;
	typedef lyx::graphics::Cache GCache;
	FmtList const loadableFormats = GCache::get().loadableFormats();
	FmtList::const_iterator it = loadableFormats.begin();
	FmtList::const_iterator const end = loadableFormats.end();

	for (; it != end; ++it) {
		string const to = *it;
		if (from == to)
			continue;

		lyx::Converter const * ptr = lyx::theConverters().getConverter(from, to);
		if (ptr)
			return ptr;
	}

	static bool first = true;
	if (first) {
		first = false;
		lyx::lyxerr << "PreviewLoader::startLoading()\n"
		       << "No converter from \"lyxpreview\" format has been "
			"defined."
		       << endl;
	}
	return 0;
}


void setAscentFractions(vector<double> & ascent_fractions,
			FileName const & metrics_file)
{
	// If all else fails, then the images will have equal ascents and
	// descents.
	vector<double>::iterator it  = ascent_fractions.begin();
	vector<double>::iterator end = ascent_fractions.end();
	fill(it, end, 0.5);

	ifstream in(metrics_file.toFilesystemEncoding().c_str());
	if (!in.good()) {
		lyx::lyxerr[lyx::Debug::GRAPHICS]
			<< "setAscentFractions(" << metrics_file << ")\n"
			<< "Unable to open file!" << endl;
		return;
	}

	bool error = false;

	int snippet_counter = 1;
	while (!in.eof() && it != end) {
		string snippet;
		int id;
		double ascent_fraction;

		in >> snippet >> id >> ascent_fraction;

		if (!in.good())
			// eof after all
			break;

		error = snippet != "Snippet";
		if (error)
			break;

		error = id != snippet_counter;
		if (error)
			break;

		*it = ascent_fraction;

		++snippet_counter;
		++it;
	}

	if (error) {
		lyx::lyxerr[lyx::Debug::GRAPHICS]
			<< "setAscentFractions(" << metrics_file << ")\n"
			<< "Error reading file!\n" << endl;
	}
}


class FindFirst : public std::unary_function<SnippetPair, bool> {
public:
	FindFirst(string const & comp) : comp_(comp) {}
	bool operator()(SnippetPair const & sp) const
	{
		return sp.first == comp_;
	}
private:
	string const comp_;
};


/// Store info on a currently executing, forked process.
class InProgress {
public:
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
	string command;
	///
	FileName metrics_file;
	///
	BitmapFile snippets;
};

typedef map<pid_t, InProgress>  InProgressProcesses;

typedef InProgressProcesses::value_type InProgressProcess;

} // namespace anon



namespace lyx {

namespace graphics {

class PreviewLoader::Impl : public boost::signals::trackable {
public:
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
	boost::signal<void(PreviewImage const &)> imageReady;

	Buffer const & buffer() const { return buffer_; }

private:
	/// Called by the Forkedcall process that generated the bitmap files.
	void finishedGenerating(pid_t, int);
	///
	void dumpPreamble(odocstream &) const;
	///
	void dumpData(odocstream &, BitmapFile const &) const;

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
	static lyx::Converter const * pconverter_;
};


lyx::Converter const * PreviewLoader::Impl::pconverter_;


//
// The public interface, defined in PreviewLoader.h
//

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


Buffer const & PreviewLoader::buffer() const
{
	return pimpl_->buffer();
}

} // namespace graphics
} // namespace lyx


// The details of the Impl
// =======================

namespace {

class IncrementedFileName {
public:
	IncrementedFileName(string const & to_format,
			    string const & filename_base)
		: to_format_(to_format), base_(filename_base), counter_(1)
	{}

	SnippetPair const operator()(string const & snippet)
	{
		ostringstream os;
		os << base_ << counter_++ << '.' << to_format_;
		string const file = os.str();

		return make_pair(snippet, FileName(file));
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
	  metrics_file(FileName(filename_base + ".metrics")),
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
		lyx::support::ForkedcallsController::get().kill(pid, 0);

	if (!metrics_file.empty())
		lyx::support::unlink(metrics_file);

	BitmapFile::const_iterator vit  = snippets.begin();
	BitmapFile::const_iterator vend = snippets.end();
	for (; vit != vend; ++vit) {
		if (!vit->second.empty())
			lyx::support::unlink(vit->second);
	}
}

} // namespace anon


namespace lyx {
namespace graphics {

PreviewLoader::Impl::Impl(PreviewLoader & p, Buffer const & b)
	: parent_(p), buffer_(b), font_scaling_factor_(0.0)
{
	font_scaling_factor_ = 0.01 * lyxrc.dpi * lyxrc.zoom *
		convert<double>(lyxrc.preview_scale_factor);

	LYXERR(Debug::GRAPHICS) << "The font scaling factor is "
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

class FindSnippet : public std::unary_function<InProgressProcess, bool> {
public:
	FindSnippet(string const & s) : snippet_(s) {}
	bool operator()(InProgressProcess const & process) const
	{
		BitmapFile const & snippets = process.second.snippets;
		BitmapFile::const_iterator beg  = snippets.begin();
		BitmapFile::const_iterator end = snippets.end();
		return find_if(beg, end, FindFirst(snippet_)) != end;
	}

private:
	string const snippet_;
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

	string const snippet = support::trim(latex_snippet);
	if (snippet.empty())
		return;

	LYXERR(Debug::GRAPHICS) << "adding snippet:\n" << snippet << endl;

	pending_.push_back(snippet);
}


namespace {

class EraseSnippet {
public:
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

	while (ipit != ipend) {
		InProgressProcesses::iterator curr = ipit++;
		if (curr->second.snippets.empty())
			in_progress_.erase(curr);
	}
}


void PreviewLoader::Impl::startLoading()
{
	if (pending_.empty() || !pconverter_)
		return;

	// Only start the process off after the buffer is loaded from file.
	if (!buffer_.fully_loaded())
		return;

	LYXERR(Debug::GRAPHICS) << "PreviewLoader::startLoading()" << endl;

	// As used by the LaTeX file and by the resulting image files
	string const directory = buffer_.temppath();

	string const filename_base = unique_filename(directory);

	// Create an InProgress instance to place in the map of all
	// such processes if it starts correctly.
	InProgress inprogress(filename_base, pending_, pconverter_->to);

	// clear pending_, so we're ready to start afresh.
	pending_.clear();

	// Output the LaTeX file.
	FileName const latexfile(filename_base + ".tex");

	// we use the encoding of the buffer
	Encoding const & enc = buffer_.params().encoding();
	odocfstream of;
	try { of.reset(enc.iconvName()); }
	catch (iconv_codecvt_facet_exception & e) {
		lyxerr << "Caught iconv exception: " << e.what()
		       << "\nUnable to create LaTeX file: "
		       << latexfile << endl;
		return;
	}

	TexRow texrow;
	OutputParams runparams(&enc);
	LaTeXFeatures features(buffer_, buffer_.params(), runparams);

	if (!openFileWrite(of, latexfile))
		return;

	if (!of) {
		LYXERR(Debug::GRAPHICS) << "PreviewLoader::startLoading()\n"
					<< "Unable to create LaTeX file\n"
					<< latexfile << endl;
		return;
	}
	of << "\\batchmode\n";
	dumpPreamble(of);
	// handle inputenc etc.
	buffer_.params().writeEncodingPreamble(of, features, texrow);
	of << "\n\\begin{document}\n";
	dumpData(of, inprogress.snippets);
	of << "\n\\end{document}\n";
	of.close();
	if (of.fail()) {
		LYXERR(Debug::GRAPHICS)  << "PreviewLoader::startLoading()\n"
					 << "File was not closed properly."
					 << endl;
		return;
	}

	// The conversion command.
	ostringstream cs;
	cs << pconverter_->command << ' ' << pconverter_->to << ' '
	   << support::quoteName(latexfile.toFilesystemEncoding()) << ' '
	   << int(font_scaling_factor_) << ' '
	   << theApp()->hexName(Color::preview) << ' '
	   << theApp()->hexName(Color::background);

	string const command = support::libScriptSearch(cs.str());

	// Initiate the conversion from LaTeX to bitmap images files.
	support::Forkedcall::SignalTypePtr
		convert_ptr(new support::Forkedcall::SignalType);
	convert_ptr->connect(bind(&Impl::finishedGenerating, this, _1, _2));

	support::Forkedcall call;
	int ret = call.startscript(command, convert_ptr);

	if (ret != 0) {
		LYXERR(Debug::GRAPHICS) << "PreviewLoader::startLoading()\n"
					<< "Unable to start process\n"
					<< command << endl;
		return;
	}

	// Store the generation process in a list of all such processes
	inprogress.pid = call.pid();
	inprogress.command = command;
	in_progress_[inprogress.pid] = inprogress;
}


void PreviewLoader::Impl::finishedGenerating(pid_t pid, int retval)
{
	// Paranoia check!
	InProgressProcesses::iterator git = in_progress_.find(pid);
	if (git == in_progress_.end()) {
		lyxerr << "PreviewLoader::finishedGenerating(): unable to find "
			"data for PID " << pid << endl;
		return;
	}

	string const command = git->second.command;
	string const status = retval > 0 ? "failed" : "succeeded";
	LYXERR(Debug::GRAPHICS) << "PreviewLoader::finishedInProgress("
				<< retval << "): processing " << status
				<< " for " << command << endl;
	if (retval > 0)
		return;

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
		FileName const & file = it->second;
		double af = ascent_fractions[metrics_counter];

		PreviewImagePtr ptr(new PreviewImage(parent_, snip, file, af));
		cache_[snip] = ptr;

		newimages.push_back(ptr);
	}

	// Remove the item from the list of still-executing processes.
	in_progress_.erase(git);

	// Tell the outside world
	std::list<PreviewImagePtr>::const_reverse_iterator
		nit  = newimages.rbegin();
	std::list<PreviewImagePtr>::const_reverse_iterator
		nend = newimages.rend();
	for (; nit != nend; ++nit) {
		imageReady(*nit->get());
	}
}


void PreviewLoader::Impl::dumpPreamble(odocstream & os) const
{
	// Why on earth is Buffer::makeLaTeXFile a non-const method?
	Buffer & tmp = const_cast<Buffer &>(buffer_);
	// Dump the preamble only.
	// We don't need an encoding for runparams since it is not used by
	// the preamble.
	OutputParams runparams(0);
	runparams.flavor = OutputParams::LATEX;
	runparams.nice = true;
	runparams.moving_arg = true;
	runparams.free_spacing = true;
	tmp.writeLaTeXSource(os, buffer_.filePath(), runparams, true, false);

	// FIXME! This is a HACK! The proper fix is to control the 'true'
	// passed to WriteStream below:
	// int InsetMathNest::latex(Buffer const &, odocstream & os,
	//                          OutputParams const & runparams) const
	// {
	//	WriteStream wi(os, runparams.moving_arg, true);
	//	par_->write(wi);
	//	return wi.line();
	// }
	os << "\n"
	   << "\\def\\lyxlock{}\n"
	   << "\n";

	// Loop over the insets in the buffer and dump all the math-macros.
	Inset & inset = buffer_.inset();
	InsetIterator it = inset_iterator_begin(inset);
	InsetIterator const end = inset_iterator_end(inset);

	for (; it != end; ++it)
		if (it->lyxCode() == Inset::MATHMACRO_CODE)
			it->latex(buffer_, os, runparams);

	// All equation labels appear as "(#)" + preview.sty's rendering of
	// the label name
	if (lyxrc.preview_hashed_labels)
		os << "\\renewcommand{\\theequation}{\\#}\n";

	// Use the preview style file to ensure that each snippet appears on a
	// fresh page.
	// Also support PDF output (automatically generated e.g. when
	// \usepackage[pdftex]{hyperref} is used.
	os << "\n"
	   << "\\newif\\ifpdf\n"
	   << "\\ifx\\pdfoutput\\undefined\n"
	   << "\\else\\ifx\\pdfoutput\\relax\n"
	   << "\\else\\ifnum0=\\pdfoutput\n"
	   << "\\else\\pdftrue\\fi\\fi\\fi\n"
	   << "\\ifpdf\n"
	   << "  \\usepackage[active,delayed,tightpage,showlabels,lyx,pdftex]{preview}\n"
	   << "\\else\n"
	   << "  \\usepackage[active,delayed,showlabels,lyx,dvips]{preview}\n"
	   << "\\fi\n"
	   << "\n";
}


void PreviewLoader::Impl::dumpData(odocstream & os,
				   BitmapFile const & vec) const
{
	if (vec.empty())
		return;

	BitmapFile::const_iterator it  = vec.begin();
	BitmapFile::const_iterator end = vec.end();

	for (; it != end; ++it) {
		// FIXME UNICODE
		os << "\\begin{preview}\n"
		   << from_utf8(it->first)
		   << "\n\\end{preview}\n\n";
	}
}

} // namespace graphics
} // namespace lyx
