#include <config.h>

#include "debug.h"
#include "support/systemcall.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsParams.h"
#include "graphics/GraphicsCacheItem.h"
#include "Lsstream.h"

#include <fstream>

#include <boost/utility.hpp>
#include <boost/bind.hpp>

using namespace std;


// built some unique filename
string canonical_name(string const & str)
{
	ostringstream os;
	for (string::const_iterator it = str.begin(); it != str.end(); ++it) 
		os << char('A' + (*it & 15)) << char('a' + (*it >> 4));
	return os.str();
}


bool preview(string const & str, grfx::GraphicPtr & graphic)
{
	string base = canonical_name(str);
	string dir  = "/tmp/lyx/";
	string file = dir + base + ".eps";
	cerr << "writing '" << str << "' to '" << file << "'\n";

	// get the cache
	grfx::GCache & gc = grfx::GCache::get();

	// look up the file
	if (gc.inCache(file)) {

		// it's already in there. Get hold of it.
		grfx::GraphicPtr gr = grfx::GraphicPtr(gc.graphic(file));

		// is it already loaded?
		if (gr->status() == grfx::Loaded) {
			cerr << "file '" << file << "' ready for display\n";
			graphic = gr;
			grfx::GParams pars;
			bool const res = graphic->image()->setPixmap(pars);
			return res;
		}

		// otherwise we have to wait again
		cerr << "file '" << file << "' not finished loading\n";
		return false;
	}

	// The real work starts.
	string const texfile = dir + base + ".tex";
	std::ofstream of(texfile.c_str());
	of << "\\batchmode"
	   << "\\documentclass{article}"
	   << "\\usepackage{amssymb}"
	   << "\\thispagestyle{empty}"
	   << "\\pdfoutput=0"
	   << "\\begin{document}"
	   << str
	   << "\\end{document}\n";
	of.close();

	string const cmd =
//		"latex " + base + ".tex ; " + "
//    "dvips -x 2500 -R -E -o " + base + ".eps " + base + ".dvi ";
	// Herbert says this is faster
		"pdflatex --interaction batchmode " + base + "; " +
		"dvips -x 2000 -R -E -o " + base + ".eps " + base + ".dvi ";
	//cerr << "calling: '" << "(cd " + dir + "; " + cmd + ")\n";
	Systemcall sc;
	sc.startscript(Systemcall::Wait, "(cd " + dir + "; " + cmd + ")");

	// now we are done, add the file to the cache
	gc.add(file);
	gc.graphic(file)->startLoading();

	// This might take a while. Wait for the next round.
	cerr << "file '" << file << "' registered\n";
	return false;
}
