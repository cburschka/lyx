/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995-2000 the LyX Team.
 *           
 *           This file Copyright 2000 Baruch Even.
 * ====================================================== */

/*
How to use it for now:
    * The lyxfunc 'graphics-insert' will insert this inset into the document.
*/

/*
Immediate tasks:
    * Add the GraphicsCache and FormatTranslator in order to get inline
        viewing of the figures.
        
*/

/*
Known BUGS:
    
    * If the image is from the clipart, and the document is moved to another
       directory, the user is screwed. Need a way to handle it.
       This amounts to a problem of when to use relative or absolute file paths
       We should probably use what the user asks to use... but when he chooses
       by the file dialog we normally get an absolute path and this may not be 
       what the user meant.
    * Bug in FileDlg class (src/filedlg.[hC]) when selecting a file and then
        pressing ok, it counts as if no real selection done. Apparently it
        when choosing a file it doesn't update the select file input line.

Current PROBLEMS:
    
    * How to support both PDF and PS output, should we do the conversion
        or should we just give the bounding box and tell latex how to do the
        conversion itself?
        I (Baruch Even) tend towards doing the conversion ourselves, otherwise
        we need to give latex quite a few translation commands and from the
        graphicx package docs it appears that it takes quite a bit of memory
        on the side of TeXing.

TODO Basics:

    * Add support for more features so that it will be useable as a drop in
      replacement to insetfig.
        * Keep aspect ratio radio button

    * Create the GraphicsCache and FormatTranslator
    * Add inline viewing of image.

TODO Before initial production release:
    * Replace insetfig everywhere
        * Read it's file format
        * Get created by all commands used to create figinset currently.
        * Search for comments of the form
            // INSET_GRAPHICS: remove this when InsetFig is thrown.
          And act upon them.

    * Pop up a dialog if the widget version is higher than what we accept.
    * Finish the basic To-do list.
    * Extract the general logic of the dialog in order to allow easier porting
        to Gnome/KDE, and put the general logic in frontends and the inherited
        platform dependent code in the appropriate dirs.
   
TODO Extended features:

    * Advanced Latex tab folder.
    * Add even more options to make it better than insetfig.
        * Support for complete control over the latex parameters for TeXperts
        * What advanced features the users want to do?
            Implement them in a non latex dependent way, but a logical way.
            LyX should translate it to latex or any other fitting format.
    * Add a way to roll the image file into the file format.
    * When loading if the image is not found in the expected place, try
       to find it in the clipart, or in the same directory with the image.
    * If the dialog had no real change from previous time, do not mark document
        as changed.
    * Keep a tab on the image file, if it changes, update the lyx view.
 */

/* NOTES:
 *
 * Intentions:
 *  This is currently a moving target, I'm trying stuff and learning what
 *  is needed and how to accomplish it, since there is no predefined goal or
 *  way to go I invent it as I go.
 *
 *  My current intention is for seperation from LaTeX, the basic needs are 
 *  resizing and rotating, displaying on screen in various depths and printing
 *  conversion of depths (independent of the display depth). For this I'll 
 *  provide a simple interface.
 *
 *  The medium level includes clipping of the image, but in a limited way.
 *
 *  For the LaTeX gurus I'll provide a complete control over the output, but
 *  this is latex dependent and guru dependent so I'd rather avoid doing this
 *  for the normal user. This stuff includes clipping, special image size
 *  specifications (\textwidth\minus 2in) which I see no way to generalize
 *  to non-latex specific way.
 *
 * Used packages:
 *  'graphicx' for the graphics inclusion.
 *  'subfigure' for the subfigures.
 *
 * Fileformat:
 *
 * Current version is 1 (inset file format version), when changing it
 * it should be changed in the Write() function when writing in one place
 * and when reading one should change the version check and the error message.
 *
 * The filename is kept in  the lyx file in a relative way, so as to allow
 * moving the document file and its images with no problem.
 *
 * Conversions:
 *  
 *  Apparently the PNG output is preferred over PDF images when doing PDF
 *  documents (i.e. prefer imagemagick eps2png over eps2pdf)
 */

/* Current Stage:
 *  Embryonic.
 *
 * PLAN:
 *  Finish basic support:
 *      Inline image viewing
 *      Get into lyx-devel as an unactivated inset for the benefit of those
 *          who really need it.
 *
 *  Do Release quality support:
 *      Allow to change display depth
 *      Make default figure instead of InsetFig
 *      Add to LyX (probably after 1.1.6 is released)
 *      
 *  Extended features:
 *      Output format conversion
 *      Print depth changes
 *      Image file tracking of changes.
 *
 *  Extended^2:
 *      Image roll-in (how? when? why?)
 *          This means to add the image inside the LyX file, usefull when
 *          transferring the file around.
 */
 
#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h"
#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
 
#include "frontends/Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "BufferView.h"
#include "Painter.h"
#include "lyx_gui_misc.h"
#include "filedlg.h"
#include "support/FileInfo.h"
#include "support/filetools.h"

#include "debug.h"


using std::ostream;
using std::endl;


// Initialize only those variables that do not have a constructor.
InsetGraphics::InsetGraphics()
#ifdef IG_OLDPARAMS    
      : use_bb(false), hiresbb(false), angle(0.0), origin(DEFAULT)
      ,keepaspectratio(false), scale(0.0), clip(false), draft(false)          
#endif     
{}

InsetGraphics::~InsetGraphics()
{
    // Emits the hide signal to the dialog connected (if any)
    hide();
}

int InsetGraphics::ascent(BufferView *, LyXFont const &) const 
{
	
	return 25;
}


int InsetGraphics::descent(BufferView *, LyXFont const &) const 
{
	// this is not true if viewport is used and clip is not.
	return 25;
}


int InsetGraphics::width(BufferView *, LyXFont const &) const 
{
    // Need to replace this with data coming from GraphicsCache
#ifdef IG_OLDPARAMS    
	if (bb.isSet()) {
		return bb.urx - bb.llx + 2;
	}
#endif    
	return 50;
}


void InsetGraphics::draw(BufferView * bv, LyXFont const & font,
			 int baseline, float & x, bool) const
{
	Painter & paint = bv->painter();

	// This will draw the graphics. As for now we only draw a
	// placeholder rectangele.
	paint.rectangle(int(x)+2, baseline - ascent(bv, font),
		       width(bv, font) - 4,
		       ascent(bv, font) + descent(bv, font));
}


void InsetGraphics::Edit(BufferView *bv, int, int, unsigned int)
{
    bv->owner()->getDialogs() -> showGraphics(this);
}


Inset::EDITABLE InsetGraphics::Editable() const
{
	return IS_EDITABLE;
}


void InsetGraphics::Write(Buffer const * buf, ostream & os) const
{
	os << "Graphics FormatVersion 1" << endl;

    params.Write(buf, os);
}

#if 0
// Baruch Even 2000-07-08

// A Thought for another way to read the file...
// The map should be a static part of the object or a static part of this
// file and should be filled during program start.
// The questions are:
// 1. Is this cleaner?
// 2. Is there no hidden performance costs?
// 
// Regarding 2 I can already see that we will have two copies of the strings
// one in the data part of the program and one in the map, but that won't be
// more than say 2K (overestimation here), there is no real benefit to put
// it in the map since there aren't that many configuration items that will
// make it a faster solution, it might just be a bit cleaner.
// (a map stores either in a hash or a kind of a balanced tree).

void InsetGraphics::Read(Buffer const * buf, LyXLex & lex)
{
    typedef map<string, enum TOKENS> ReadActionMap;
    static ReadActionMap const readMap;

    bool finished = false;

    while (lex.IsOK() && !finished) {
        lex.next();

        string const token = lex.GetString();
        lyxerr.debug() << "Token: '" << token << '\'' << endl;

        if (token.empty())
            continue;
        
        ReadActionMap::const_iterator it =
            readMap.find(token);

        if (it == readMap.end()) {
            lyxerr << "Unknown keyword, skipping." << endl;
            continue;
        }
        
        switch (it.second) {
        case FILENAME_TOKEN:
            break;
        case VERSION_TOKEN:
            break;
        default:
            break;
        }

            
    }
}
#endif

void InsetGraphics::Read(Buffer const * buf, LyXLex & lex) 
{
    bool finished = false;
    
    while (lex.IsOK() && !finished) {
        lex.next();
        
        string const token = lex.GetString();
        lyxerr.debug() << "Token: '" << token << '\'' << endl;

        if (token.empty()) {
            continue;
        } else if (token == "\\end_inset") {
            finished = true;
        } else if (token == "FormatVersion") {
            lex.next();
            int version = lex.GetInteger();
            if (version > 1)
                lyxerr 
                    << "This document was created with a newer Graphics widget"
                       ", You should use a newer version of LyX to read this"
                       " file."
                    << endl;
            // TODO: Possibly open up a dialog?
        } else {
            if (! params.Read(buf, lex, token))
                lyxerr << "Unknown token, " << token << ",skipping." << endl;
        }
    }
    
    updateInset();
}

static void formatResize(ostream & os, char const *key,
        InsetGraphicsParams::Resize resizeType, double size)
{
    switch (resizeType) {
    case InsetGraphicsParams::DEFAULT_SIZE:
        break;
    
    case InsetGraphicsParams::CM:
        os << key << '=' << size << "cm,";
        break;
            
    case InsetGraphicsParams::INCH:
        os << key << '=' << size << "in,";
        break;
            
    case InsetGraphicsParams::PERCENT_PAGE:
        os << key << '=' << size/100 << "\\text" << key << ',';
        break;
            
    case InsetGraphicsParams::PERCENT_COLUMN:
        os << key << '=' << size/100 << "\\column" << key << ',';
        break;
            
    }
}

int InsetGraphics::Latex(Buffer const *buf, ostream & os,
			 bool /*fragile*/, bool/*fs*/) const
{
	// MISSING: We have to decide how to do the order of the options
	// that is dependent of order, like witdth, height, angle. Should
	// we rotate before scale? Should we let the user decide?
	// bool rot_before_scale; ?

    // (BE) As a first step we should do a scale before rotate since this is
    // more like the natural thought of how to do it.
    // (BE) I believe that a priority list presented to the user with
    // a default order would be the best, though it would be better to
    // hide such a thing in an "Advanced options" dialog.
    // (BE) This should go an advanced LaTeX options dialog.

    // If there is no file specified, just output a message about it in
    // the latex output.
    if (params.filename.empty()) {
        os << "\\fbox{\\rule[-0.5in]{0pt}{1in}"
           << _("empty figure path")
           << '}'
           << endl;

        return 1;
    }

    // Calculate the options part of the command, we must do it to a string
    // stream since we might have a trailing comma that we would like to remove
    // before writing it to the output stream.
#ifdef HAVE_SSTREAM
	std::ostringstream options;
#else
	ostrstream options;
#endif
    
    formatResize(options, "width", params.widthResize, params.widthSize);
    formatResize(options, "height", params.heightResize, params.heightSize);

    if (params.rotateAngle != 0) {
        options << "angle="
            << params.rotateAngle << ',';
    }
    
#ifdef IG_OLDPARAMS    
	if (bb.isSet() && use_bb) {
		options << "bb="
			<< bb.llx << ' ' << bb.lly << ' '
			<< bb.urx << ' ' << bb.ury << ',';
	}
	if (hiresbb) {
		options << "hiresbb,";
	}
	if (viewport.isSet()) {
		options << "viewport="
			<< viewport.llx << ' ' << viewport.lly << ' '
			<< viewport.urx << ' ' << viewport.ury << ',';
	}
	if (trim.isSet()) {
		options << "trim="
			<< trim.llx << ' ' << trim.lly << ' '
			<< trim.urx << ' ' << trim.ury << ',';
	}
	if (natheight.value() != 0) {
		options << "natheight=" << natheight.asString() << ',';
	}
	if (natwidth.value() != 0) {
		options << "natwidth=" << natwidth.asString() << ',';
	}
	if (angle != 0.0) {
		options << "angle=" << angle << ',';
	}
	if (origin != DEFAULT) {
		switch(origin) {
		case DEFAULT: break;
		case LEFTTOP:
			options << "origin=lt,";
			break;
		case LEFTCENTER:
			options << "origin=lc,";
			break;
		case LEFTBASELINE:
			options << "origin=lB,";
			break;
		case LEFTBOTTOM:
			options << "origin=lb,";
			break;
		case CENTERTOP:
			options << "origin=ct,";
			break;
		case CENTER:
			options << "origin=c,";
			break;
		case CENTERBASELINE:
			options << "origin=cB,";
			break;
		case CENTERBOTTOM:
			options << "origin=cb,";
			break;
		case RIGHTTOP:
			options << "origin=rt,";
			break;
		case RIGHTCENTER:
			options << "origin=rc,";
			break;
		case RIGHTBASELINE:
			options << "origin=rB,";
			break;
		case RIGHTBOTTOM:
			options << "origin=rb,";
			break;
		}
	}
	if (g_width.value() != 0) {
		options << "width=" << g_width.asString() << ',';
	}
	if (g_height.value() != 0) {
		options << "height=" << g_height.asString() << ',';
	}
	if (totalheight.value() != 0) {
		options << "totalheight=" << totalheight.asString() << ',';
	}
	if (keepaspectratio) {
		options << "keepaspectratio,";
	}
	if (scale != 0.0) {
		options << "scale=" << scale << ',';
	}
	if (clip) {
		options << "clip,";
	}
	if (draft) {
		options << "draft,";
	}
	if (!type.empty()) {
		options << "type=" << type << ',';
	
        // These should be present only when type is used.
	    if (!ext.empty()) {
		    options << "ext=" << type << ',';
	    }
	    if (!read.empty()) {
		    options << "read=" << type << ',';
	    }
	    if (!command.empty()) {
		    options << "command=" << type << ',';
	    }
    }
#endif

#ifdef HAVE_SSTREAM
	string opts(options.str().c_str());
#else
	options << '\0';
	char * tmp = options.str();
	string opts(tmp);
	delete [] tmp;
#endif
	opts = strip(opts, ',');

    
    // If it's not an inline image, surround it with the centering paragraph.
    if (! params.inlineFigure) {
        os << endl 
            << "\\vspace{0.3cm}" << endl
            << "{\\par\\centering ";
    }
   
    // Do we want subcaptions?
    if (params.subcaption) {
        os << "\\subfigure[" << params.subcaptionText << "]{";
    }
    
	// We never used the starred form, we use the "clip" option instead.
    os << "\\includegraphics";

	if (!opts.empty()) {
		os << '[' << opts << ']';
	}

    // Make the filename relative to the lyx file
    string filename = MakeRelPath(params.filename, OnlyPath(buf->fileName()));

    // and remove the extension so the LaTeX will use whatever is
    // appropriate (when there are several versions in different formats)
    filename = ChangeExtension(filename, string());

	os << '{' << filename << '}';

    // Do we want a subcaption?
    if (params.subcaption) {
        // Close the subcaption command
        os << '}';
    }
    
    // Is this an inline graphics?
    if (!params.inlineFigure) {
        os  << " \\par}" << endl
            << "\\vspace{0.3cm}" << endl;
    }

    // How do we decide to what format should we export?
//    cachehandle->export(ImageType::EPS);
//    cachehandle->export(ImageType::PNG);
        
	return 1;
}


int InsetGraphics::Ascii(Buffer const *, ostream &) const
{
    // No graphics in ascii output.
	return 0;
}


int InsetGraphics::Linuxdoc(Buffer const *, ostream &) const
{
    // No graphics in LinuxDoc output. Should check how/what to add.
	return 0;
}


int InsetGraphics::DocBook(Buffer const *, ostream &) const
{
    // No graphics in DocBook output. Should check how/what to add.
	return 0;
}


void InsetGraphics::Validate(LaTeXFeatures & features) const
{
    // If we have no image, we should not require anything.
    if (params.filename.empty())
        return;

	features.graphicx = true;

    if (params.subcaption)
        features.subfigure = true;
}

// Update the inset after parameters changed (read from file or changed in
// dialog.
void InsetGraphics::updateInset()
{
    // If file changed...
	//graphicscache.addFile(params.filename);
	//bb = graphicscache.getBB(params.filename);
	//pixmap = graphicscache.getPixmap(params.filename);
}

bool InsetGraphics::setParams(InsetGraphicsParams const & params)
{
    // TODO: Make it return true only when the data has been changed.
    // for this to work we still need to implement operator == in 
    // InsetGraphicsParams
    if (this->params == params)
        return false;

    // Copy the new parameters.
    this->params = params;

    // Update the inset with the new parameters.
    updateInset();
   
    // We have changed data, report it.
    return true;
}

InsetGraphicsParams InsetGraphics::getParams() const
{
    return params;
}

Inset * InsetGraphics::Clone() const
{
    InsetGraphics * newInset = new InsetGraphics;
    newInset->setParams(getParams());
    
	return newInset;
}
