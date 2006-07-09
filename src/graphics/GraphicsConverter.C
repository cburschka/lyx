/**
 * \file GraphicsConverter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsConverter.h"

#include "converter.h"
#include "debug.h"
#include "format.h"

#include "support/filetools.h"
#include "support/forkedcallqueue.h"
#include "support/convert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"

#include <boost/bind.hpp>

#include <sstream>
#include <fstream>

namespace support = lyx::support;

using support::changeExtension;
using support::Forkedcall;
using support::ForkedCallQueue;
using support::libFileSearch;
using support::libScriptSearch;
using support::onlyPath;
using support::onlyFilename;
using support::quoteName;
using support::subst;
using support::tempName;
using support::unlink;

using std::endl;
using std::ostream;
using std::ostringstream;
using std::string;


namespace lyx {
namespace graphics {

class Converter::Impl : public boost::signals::trackable {
public:
	///
	Impl(string const &, string const &, string const &, string const &);

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
	string script_file_;
	///
	string to_file_;
	///
	bool valid_process_;
	///
	bool finished_;
};


bool Converter::isReachable(string const & from_format_name,
			    string const & to_format_name)
{
	return converters.isReachable(from_format_name, to_format_name);
}


Converter::Converter(string const & from_file,   string const & to_file_base,
		     string const & from_format, string const & to_format)
	: pimpl_(new Impl(from_file, to_file_base, from_format, to_format))
{}


// Empty d-tor out-of-line to keep boost::scoped_ptr happy.
Converter::~Converter()
{}


void Converter::startConversion() const
{
	pimpl_->startConversion();
}


boost::signals::connection Converter::connect(slot_type const & slot) const
{
	return pimpl_->finishedConversion.connect(slot);
}


string const & Converter::convertedFile() const
{
	static string const empty;
	return pimpl_->finished_ ? pimpl_->to_file_ : empty;
}

} // namespace graphics
} // namespace lyx


//------------------------------
// Implementation details follow
//------------------------------

namespace {

/** Build the conversion script, returning true if able to build it.
 *  The script is output to the ostringstream 'script'.
 */
bool build_script(string const & from_file, string const & to_file_base,
		  string const & from_format, string const & to_format,
		  ostream & script);

} // namespace anon


namespace lyx {
namespace graphics {

Converter::Impl::Impl(string const & from_file,   string const & to_file_base,
		      string const & from_format, string const & to_format)
	: valid_process_(false), finished_(false)
{
	lyxerr[Debug::GRAPHICS] << "Converter c-tor:\n"
		<< "\tfrom_file:      " << from_file
		<< "\n\tto_file_base: " << to_file_base
		<< "\n\tfrom_format:  " << from_format
		<< "\n\tto_format:    " << to_format << endl;

	// The converted image is to be stored in this file (we do not
	// use ChangeExtension because this is a basename which may
	// nevertheless contain a '.')
	to_file_ = to_file_base + '.' +  formats.extension(to_format);

	// The conversion commands are stored in a stringstream
	ostringstream script;
	bool const success = build_script(from_file, to_file_base,
					  from_format, to_format, script);

	if (!success) {
		script_command_ =
			support::os::python() + ' ' +
			quoteName(libFileSearch("scripts", "convertDefault.py")) +
			' ' +
			quoteName((from_format.empty() ? "" : from_format + ':') + from_file) +
			' ' +
			quoteName(to_format + ':' + to_file_);

		lyxerr[Debug::GRAPHICS]
			<< "\tNo converter defined! I use convertDefault.py\n\t"
			<< script_command_ << endl;

	} else {

		lyxerr[Debug::GRAPHICS] << "\tConversion script:"
			<< "\n--------------------------------------\n"
			<< script.str()
			<< "\n--------------------------------------\n";

		// Output the script to file.
		static int counter = 0;
		script_file_ = onlyPath(to_file_base) + "lyxconvert" +
			convert<string>(counter++) + ".py";

		std::ofstream fs(script_file_.c_str());
		if (!fs.good()) {
			lyxerr << "Unable to write the conversion script to \""
			       << script_file_ << '\n'
			       << "Please check your directory permissions."
			       << std::endl;
			return;
		}

		fs << script.str();
		fs.close();

		// The command needed to run the conversion process
		// We create a dummy command for ease of understanding of the
		// list of forked processes.
		// Note: 'sh ' is absolutely essential, or execvp will fail.
		script_command_ = support::os::python() + ' ' +
			quoteName(script_file_) + ' ' +
			quoteName(onlyFilename(from_file)) + ' ' +
			quoteName(to_format);
	}
	// All is ready to go
	valid_process_ = true;
}


void Converter::Impl::startConversion()
{
	if (!valid_process_) {
		converted(0, 1);
		return;
	}

	Forkedcall::SignalTypePtr
		ptr = ForkedCallQueue::get().add(script_command_);

	ptr->connect(boost::bind(&Impl::converted, this, _1, _2));

}

void Converter::Impl::converted(pid_t /* pid */, int retval)
{
	if (finished_)
		// We're done already!
		return;

	finished_ = true;
	// Clean-up behind ourselves
	unlink(script_file_);

	if (retval > 0) {
		unlink(to_file_);
		to_file_.erase();
		finishedConversion(false);
	} else {
		finishedConversion(true);
	}
}

} // namespace graphics
} // namespace lyx

namespace {

string const move_file(string const & from_file, string const & to_file)
{
	if (from_file == to_file)
		return string();

	ostringstream command;
	command << "fromfile = " << from_file << "\n"
		<< "tofile = "   << to_file << "\n\n"
		<< "try:\n"
		<< "  os.rename(fromfile, tofile)\n"
		<< "except:\n"
		<< "  import shutil\n"
		<< "  try:\n"
		<< "    shutil.copy(fromfile, tofile)\n"
		<< "  except:\n"
		<< "    sys.exit(1)\n"
		<< "  unlinkNoThrow(fromfile)\n";

	return command.str();
}


/*
A typical script looks like:

#!/usr/bin/env python -tt
import os, sys

def unlinkNoThrow(file):
  ''' remove a file, do not throw if error occurs '''
  try:
    os.unlink(file)
  except:
    pass

infile = '/home/username/Figure3a.eps'
infile_base = '/home/username/Figure3a'
outfile = '/tmp/lyx_tmpdir12992hUwBqt/gconvert0129929eUBPm.pdf'

if os.system(r'epstopdf ' + '"' + infile + '"' + ' --output ' + '"' + outfile + '"' + '') != 0:
  unlinkNoThrow(outfile)
  sys.exit(1)

if not os.path.isfile(outfile):
  if os.path.isfile(outfile + '.0'):
    os.rename(outfile + '.0', outfile)
    import glob
    for file in glob.glob(outfile + '.?'):
      unlinkNoThrow(file)
  else:
    sys.exit(1)

fromfile = outfile
tofile = '/tmp/lyx_tmpdir12992hUwBqt/Figure3a129927ByaCl.ppm'

try:
  os.rename(fromfile, tofile)
except:
  import shutil
  try:
    shutil.copy(fromfile, tofile)
  except:
    sys.exit(1)
  unlinkNoThrow(fromfile)

*/
bool build_script(string const & from_file,
		  string const & to_file_base,
		  string const & from_format,
		  string const & to_format,
		  ostream & script)
{
	lyxerr[Debug::GRAPHICS] << "build_script ... ";
	typedef Converters::EdgePath EdgePath;

	if (from_format.empty())
		return false;

	script << "#!/usr/bin/env python -tt\n"
	          "import os, sys\n\n"
	          "def unlinkNoThrow(file):\n"
	          "  ''' remove a file, do not throw if an error occurs '''\n"
	          "  try:\n"
	          "    os.unlink(file)\n"
	          "  except:\n"
	          "    pass\n\n";

	// we do not use ChangeExtension because this is a basename
	// which may nevertheless contain a '.'
	string const to_file = to_file_base + '.'
		+ formats.extension(to_format);

	if (from_format == to_format) {
		script << move_file(quoteName(from_file), quoteName(to_file));
		lyxerr[Debug::GRAPHICS] << "ready (from == to)" << endl;
		return true;
	}

	EdgePath edgepath = converters.getPath(from_format, to_format);

	if (edgepath.empty()) {
		lyxerr[Debug::GRAPHICS] << "ready (edgepath.empty())" << endl;
		return false;
	}

	// Create a temporary base file-name for all intermediate steps.
	// Remember to remove the temp file because we only want the name...
	static int counter = 0;
	string const tmp = "gconvert" + convert<string>(counter++);
	string const to_base = tempName(string(), tmp);
	unlink(to_base);

	string outfile = from_file;

	// The conversion commands may contain these tokens that need to be
	// changed to infile, infile_base, outfile respectively.
	string const token_from("$$i");
	string const token_base("$$b");
	string const token_to("$$o");

	EdgePath::const_iterator it  = edgepath.begin();
	EdgePath::const_iterator end = edgepath.end();

	for (; it != end; ++it) {
		::Converter const & conv = converters.get(*it);

		// Build the conversion command
		string const infile      = outfile;
		string const infile_base = changeExtension(infile, string());
		outfile = changeExtension(to_base, conv.To->extension());

		// Store these names in the shell script
		script << "infile = "      << quoteName(infile) << '\n'
		       << "infile_base = " << quoteName(infile_base) << '\n'
		       << "outfile = "     << quoteName(outfile) << '\n';

		string command = conv.command;
		command = subst(command, token_from, "' + '\"' + infile + '\"' + '");
		command = subst(command, token_base, "' + '\"' + infile_base + '\"' + '");
		command = subst(command, token_to,   "' + '\"' + outfile + '\"' + '");
		command = libScriptSearch(command);

		// Store in the shell script
		script << "\nif os.system(r'" << command << "') != 0:\n";

		// Test that this was successful. If not, remove
		// ${outfile} and exit the shell script
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

		// Delete the infile, if it isn't the original, from_file.
		if (infile != from_file)
			script << "unlinkNoThrow(infile)\n\n";
	}

	// Move the final outfile to to_file
	script << move_file("outfile", quoteName(to_file));
	lyxerr[Debug::GRAPHICS] << "ready!" << endl;

	return true;
}

} // namespace anon
