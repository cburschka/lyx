
#include <config.h>

#include "formula.h"
#include "debug.h"
#include "frontends/Painter.h"
#include "support/systemcall.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsImageXPM.h"

#include <fstream>
#include <map>


using namespace std;


namespace {

	typedef map<string, grfx::ImagePtr> previews_map;

	// cache for computed previews
	previews_map thePreviews;

	// cache for scedule previews
	vector<string> theSchedule;
}


void imageLoaded()
{
}


grfx::ImagePtr preview(string const & str)
{
	// do we already have access to a rendered version?
	previews_map::const_iterator it = thePreviews.find(str);
	if (it != thePreviews.end())
		return it->second;	

	// constructing new item
	grfx::ImagePtr & im = thePreviews[str];

	lyxerr << "writing: " << str << endl;
	std::ofstream of("/tmp/previewlyx.tex");
	of << "\\documentclass{article}"
	   << "\\usepackage{amssymb}"
	   << "\\thispagestyle{empty}"
	   << "\\begin{document}"
	   << str
	   << "\\end{document}\n";
	of.close();

	Systemcall sc1;
	sc1.startscript(Systemcall::Wait,
		"(cd /tmp ; latex previewlyx.tex ; dvips previewlyx.dvi)");

	Systemcall sc2;
	sc2.startscript(Systemcall::Wait,
		"(cd /tmp ; convert previewlyx.ps previewlyx.xpm)");

	//grfx::SignalLoadTypePtr on_finish;
	//on_finish.reset(new SignalLoadType);
	//on_finish->connect(SigC::slot(this, &imageLoaded));

	// load image
	XpmImage * xpm_image = new XpmImage;
	int const success =
		XpmReadFileToXpmImage("/tmp/previewlyx.ps", xpm_image, 0);

	switch (success) {
	case XpmOpenFailed:
		lyxerr[Debug::GRAPHICS]
			<< "No XPM image file found." << std::endl;
		break;

	case XpmFileInvalid:
		lyxerr[Debug::GRAPHICS]
			<< "File format is invalid" << std::endl;
		break;

	case XpmNoMemory:
		lyxerr[Debug::GRAPHICS]
			<< "Insufficient memory to read in XPM file"
			<< std::endl;
		break;
	}

	if (success != XpmSuccess) {
		XpmFreeXpmImage(xpm_image);
		delete xpm_image;

		lyxerr[Debug::GRAPHICS]
			<< "Error reading XPM file '"
			<< XpmGetErrorString(success) << "'"
			<< std::endl;
	} else {
		//grfx::GImageXPM * xim = static_cast<grfx::GImageXPM *>(im.get());
		//xim->image_.reset(*xpm_image);
	}

	return im;
}

