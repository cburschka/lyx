/**
 * \file GraphicsConverter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsConverter.h"

#include "Converter.h"
#include "Format.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/ForkedCalls.h"
#include "support/lstrings.h"
#include "support/os.h"

#include "support/bind.h"
#include "support/TempFile.h"

#include <sstream>
#include <fstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace graphics {

class Converter::Impl : public boost::signals::trackable {
public:
	///
	Impl(FileName const &, string const &, string const &, string const &);

	///
	void startConversion();

	/** This method is connected to a signal passed to the forked call
	 *  class, passing control back here when the conversion is completed.
	 *  Cleans-up the temporary files, emits the finishedConversion
	 *  signal and removes the Converter from the list of all processes.
	 */
	void converted(pid_t pid, int retval);

	/** At the end of the conversion process inform the outside world
	 *  by emitting a signal.
	 */
	typedef boost::signal<void(bool)> SignalType;
	///
	SignalType finishedConversion;

	///
	string script_command_;
	///
	FileName script_file_;
	///
	FileName to_file_;
	///
	bool valid_process_;
	///
	bool finished_;
};


bool Converter::isReachable(string const & from_format_name,
			    string const & to_format_name)
{
	return theConverters().isReachable(from_format_name, to_format_name);
}


Converter::Converter(FileName const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format)
	: pimpl_(new Impl(from_file, to_file_base, from_format, to_format))
{}


Converter::~Converter()
{
	delete pimpl_;
}


void Converter::startConversion() const
{
	pimpl_->startConversion();
}


boost::signals::connection Converter::connect(slot_type const & slot) const
{
	return pimpl_->finishedConversion.connect(slot);
}


FileName const & Converter::convertedFile() const
{
	static FileName const empty;
	return pimpl_->finished_ ? pimpl_->to_file_ : empty;
}

/** Build the conversion script.
 *  The script is output to the stream \p script.
 */
static void build_script(string const & from_file, string const & to_file_base,
		  string const & from_format, string const & to_format,
		  ostream & script);


Converter::Impl::Impl(FileName const & from_file, string const & to_file_base,
		      string const & from_format, string const & to_format)
	: valid_process_(false), finished_(false)
{
	LYXERR(Debug::GRAPHICS, "Converter c-tor:\n"
		<< "\tfrom_file:      " << from_file
		<< "\n\tto_file_base: " << to_file_base
		<< "\n\tfrom_format:  " << from_format
		<< "\n\tto_format:    " << to_format);

	// The converted image is to be stored in this file (we do not
	// use ChangeExtension because this is a basename which may
	// nevertheless contain a '.')
	to_file_ = FileName(to_file_base + '.' +  formats.extension(to_format));

	// The conversion commands are stored in a stringstream
	ostringstream script;
	build_script(from_file.toFilesystemEncoding(),
		     to_file_.toFilesystemEncoding(),
		     from_format, to_format, script);
	LYXERR(Debug::GRAPHICS, "\tConversion script:"
		   "\n--------------------------------------\n"
		<< script.str()
		<< "\n--------------------------------------\n");

	// Output the script to file.
	// FIXME THREAD
	static int counter = 0;
	script_file_ = FileName(onlyPath(to_file_base) + "lyxconvert" +
		convert<string>(counter++) + ".py");

	ofstream fs(script_file_.toFilesystemEncoding().c_str());
	if (!fs.good()) {
		lyxerr << "Unable to write the conversion script to \""
		       << script_file_ << '\n'
		       << "Please check your directory permissions."
		       << endl;
		return;
	}

	fs << script.str();
	fs.close();

	// The command needed to run the conversion process
	// We create a dummy command for ease of understanding of the
	// list of forked processes.
	// Note: 'python ' is absolutely essential, or execvp will fail.
	script_command_ = os::python() + ' ' +
		quoteName(script_file_.toFilesystemEncoding()) + ' ' +
		quoteName(onlyFileName(from_file.toFilesystemEncoding())) + ' ' +
		quoteName(to_format);
	// All is ready to go
	valid_process_ = true;
}


void Converter::Impl::startConversion()
{
	if (!valid_process_) {
		converted(0, 1);
		return;
	}

	ForkedCall::SignalTypePtr ptr =
		ForkedCallQueue::add(script_command_);
	ptr->connect(bind(&Impl::converted, this, _1, _2));
}


void Converter::Impl::converted(pid_t /* pid */, int retval)
{
	if (finished_)
		// We're done already!
		return;

	finished_ = true;
	// Clean-up behind ourselves
	script_file_.removeFile();

	if (retval > 0) {
		to_file_.removeFile();
		to_file_.erase();
		finishedConversion(false);
	} else {
		finishedConversion(true);
	}
}


static string const move_file(string const & from_file, string const & to_file)
{
	if (from_file == to_file)
		return string();

	ostringstream command;
	command << "fromfile = " << from_file << "\n"
		<< "tofile = "   << to_file << "\n\n"
		<< "try:\n"
		<< "  os.rename(fromfile, tofile)\n"
		<< "except:\n"
		<< "  try:\n"
		<< "    shutil.copy(fromfile, tofile)\n"
		<< "  except:\n"
		<< "    sys.exit(1)\n"
		<< "  unlinkNoThrow(fromfile)\n";

	return command.str();
}


static void build_conversion_command(string const & command, ostream & script)
{
	// Store in the python script
	script << "\nif os.system(r'" << command << "') != 0:\n";

	// Test that this was successful. If not, remove
	// ${outfile} and exit the python script
	script << "  unlinkNoThrow(outfile)\n"
	       << "  sys.exit(1)\n\n";

	// Test that the outfile exists.
	// ImageMagick's convert will often create ${outfile}.0,
	// ${outfile}.1.
	// If this occurs, move ${outfile}.0 to ${outfile}
	// and delete ${outfile}.? (ignore errors)
	script << "if not os.path.isfile(outfile):\n"
		  "  if os.path.isfile(outfile + '.0'):\n"
		  "    os.rename(outfile + '.0', outfile)\n"
		  "    import glob\n"
		  "    for file in glob.glob(outfile + '.?'):\n"
		  "      unlinkNoThrow(file)\n"
		  "  else:\n"
		  "    sys.exit(1)\n\n";

	// Delete the infile
	script << "if infile != outfile:\n"
		  "  unlinkNoThrow(infile)\n\n";
}


static string const strip_digit(string const & format)
{
	// Strip trailing digits from format names e.g. "pdf6" -> "pdf"
	return format.substr(0, format.find_last_not_of("0123456789") + 1);
}


static void build_script(string const & from_file,
		  string const & to_file,
		  string const & from_format,
		  string const & to_format,
		  ostream & script)
{
	LASSERT(from_format != to_format, return);
	LYXERR(Debug::GRAPHICS, "build_script ... ");
	typedef Graph::EdgePath EdgePath;

	script << "#!/usr/bin/env python\n"
		  "# -*- coding: utf-8 -*-\n"
		  "import os, shutil, sys\n\n"
		  "def unlinkNoThrow(file):\n"
		  "  ''' remove a file, do not throw if an error occurs '''\n"
		  "  try:\n"
		  "    os.unlink(file)\n"
		  "  except:\n"
		  "    pass\n\n";

	EdgePath const edgepath = from_format.empty() ?
		EdgePath() :
		theConverters().getPath(from_format, to_format);

	// Create a temporary base file-name for all intermediate steps.
	// Remember to remove the temp file because we only want the name...
	// FIXME THREAD
	static int counter = 0;
	string const tmp = "gconvert" + convert<string>(counter++);
	TempFile tempfile(tmp);
	tempfile.setAutoRemove(false);
	string const to_base = tempfile.name().toFilesystemEncoding();

	// Create a copy of the file in case the original name contains
	// problematic characters like ' or ". We can work around that problem
	// in python, but the converters might be shell scripts and have more
	// troubles with it.
	string outfile = addExtension(to_base, getExtension(from_file));
	script << "infile = "
			<< quoteName(from_file, quote_python)
			<< "\n"
		  "outfile = "
			<< quoteName(outfile, quote_python) << "\n"
		  "shutil.copy(infile, outfile)\n";

	// Some converters (e.g. lilypond) can only output files to the
	// current directory, so we need to change the current directory.
	// This has the added benefit that all other files that may be
	// generated by the converter are deleted when LyX closes and do not
	// clutter the real working directory.
	script << "os.chdir("
	       << quoteName(onlyPath(outfile)) << ")\n";

	if (edgepath.empty()) {
		// Either from_format is unknown or we don't have a
		// converter path from from_format to to_format, so we use
		// the default converter.
		script << "infile = outfile\n"
		       << "outfile = "
		       << quoteName(to_file, quote_python) << "\n";

		ostringstream os;
		os << os::python() << ' '
		   << commandPrep("$$s/scripts/convertDefault.py") << ' ';
		if (!from_format.empty())
			os << strip_digit(from_format) << ':';
		// The extra " quotes around infile and outfile are needed
		// because the filename may contain spaces and it is used
		// as argument of os.system().
		os << "' + '\"' + infile + '\"' + ' "
		   << strip_digit(to_format) << ":' + '\"' + outfile + '\"' + '";
		string const command = os.str();

		LYXERR(Debug::GRAPHICS,
			"\tNo converter defined! I use convertDefault.py\n\t"
			<< command);

		build_conversion_command(command, script);
	}

	// The conversion commands may contain these tokens that need to be
	// changed to infile, infile_base, outfile and output directory respectively.
	string const token_from  = "$$i";
	string const token_base  = "$$b";
	string const token_to    = "$$o";
	string const token_todir = "$$d";

	EdgePath::const_iterator it  = edgepath.begin();
	EdgePath::const_iterator end = edgepath.end();

	for (; it != end; ++it) {
		lyx::Converter const & conv = theConverters().get(*it);

		// Build the conversion command
		string const infile      = outfile;
		string const infile_base = changeExtension(infile, string());
		outfile = conv.result_file.empty()
			? addExtension(to_base, conv.To->extension())
			: addName(subst(conv.result_dir,
					token_base, infile_base),
				  subst(conv.result_file,
					token_base, onlyFileName(infile_base)));

		// If two formats share the same extension we may get identical names
		if (outfile == infile && conv.result_file.empty()) {
			TempFile tempfile(tmp);
			tempfile.setAutoRemove(false);
			string const new_base = tempfile.name().toFilesystemEncoding();
			outfile = addExtension(new_base, conv.To->extension());
		}

		// Store these names in the python script
		script << "infile = "
				<< quoteName(infile, quote_python) << "\n"
			  "infile_base = "
				<< quoteName(infile_base, quote_python) << "\n"
			  "outfile = "
				<< quoteName(outfile, quote_python) << "\n"
			  "outdir  = os.path.dirname(outfile)\n" ;

		// See comment about extra " quotes above (although that
		// applies only for the first loop run here).
		string command = conv.command;
		command = subst(command, token_from,  "' + '\"' + infile + '\"' + '");
		command = subst(command, token_base,  "' + '\"' + infile_base + '\"' + '");
		command = subst(command, token_to,    "' + '\"' + outfile + '\"' + '");
		command = subst(command, token_todir, "' + '\"' + outdir + '\"' + '");

		build_conversion_command(command, script);
	}

	// Move the final outfile to to_file
	script << move_file("outfile", quoteName(to_file, quote_python));
	LYXERR(Debug::GRAPHICS, "ready!");
}

} // namespace graphics
} // namespace lyx
