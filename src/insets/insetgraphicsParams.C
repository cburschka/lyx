// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "insetgraphicsParams.h"

#include "support/translator.h"
#include "support/filetools.h"

#ifdef ENABLE_ASSERTIONS
#include "support/LAssert.h"
#endif

/// This variable keeps a tab on whether the translator was set with the
/// translations.
static bool translatorsSet = false;

/// This is the translator between the Resize enum and corresponding lyx
/// file strings.
static Translator<InsetGraphicsParams::Resize, string>
    resizeTranslator(InsetGraphicsParams::DEFAULT_SIZE, "default");

/// This is the translator between the Origin enum and corresponding lyx
/// file strings.
static Translator<InsetGraphicsParams::Origin, string>
    originTranslator(InsetGraphicsParams::DEFAULT, "default");

/// This is the translator between the Display enum and corresponding lyx
/// file strings.
static Translator<InsetGraphicsParams::DisplayType, string>
    displayTranslator(InsetGraphicsParams::MONOCHROME, "monochrome");



InsetGraphicsParams::InsetGraphicsParams()
{
    init();

    // Set translators
    if (! translatorsSet) {
        translatorsSet = true;

        // Fill the resize translator
        resizeTranslator.addPair(DEFAULT_SIZE, "default");
        resizeTranslator.addPair(CM, "cm");
        resizeTranslator.addPair(INCH, "inch");
        resizeTranslator.addPair(PERCENT_PAGE, "percentOfPage");
        resizeTranslator.addPair(PERCENT_COLUMN, "percentOfColumn");

        // Fill the origin translator
        originTranslator.addPair(DEFAULT, "default");
        originTranslator.addPair(LEFTTOP, "leftTop");
        originTranslator.addPair(LEFTCENTER, "leftCenter");
        originTranslator.addPair(LEFTBASELINE, "leftBaseLine");
        originTranslator.addPair(LEFTBOTTOM, "leftBottom");
        originTranslator.addPair(CENTERTOP, "centerTop");
        originTranslator.addPair(CENTER, "center");
        originTranslator.addPair(CENTERBASELINE, "centerBaseLine");
        originTranslator.addPair(CENTERBOTTOM, "centerBottom");
        originTranslator.addPair(RIGHTTOP, "rightTop");
        originTranslator.addPair(RIGHTCENTER, "rightCenter");
        originTranslator.addPair(RIGHTBASELINE, "rightBaseLine");
        originTranslator.addPair(RIGHTBOTTOM, "rightBottom");
        originTranslator.addPair(REFERENCE_POINT, "referencePoint");
        
        // Fill the display translator
        displayTranslator.addPair(MONOCHROME, "monochrome");
        displayTranslator.addPair(GRAYSCALE, "grayscale");
        displayTranslator.addPair(COLOR, "color");
        displayTranslator.addPair(NONE, "none");
    }
    
}


InsetGraphicsParams::InsetGraphicsParams(InsetGraphicsParams const & igp)
{
    // I decided to skip the initialization since the copy will overwrite
    // everything anyway.
//    init();
    copy(igp);
}

InsetGraphicsParams const & 
InsetGraphicsParams::operator=(InsetGraphicsParams const &params)
{
    // Are we assigning the object into itself?
    if (this == &params)
        return *this;
    
    copy(params);    
    return *this;
}

void InsetGraphicsParams::init()
{
    subcaptionText = filename = string();
    display = MONOCHROME;
    inlineFigure = false;
    subcaption = false;
    keepAspectRatio = true;
    widthResize = DEFAULT_SIZE;
    widthSize = 0.0;
    heightResize = DEFAULT_SIZE;
    heightSize = 0.0;
    rotateOrigin = DEFAULT;
    rotateAngle = 0;

#ifdef ENABLE_ASSERTION
    testInvariant();
#endif
}

void InsetGraphicsParams::copy(InsetGraphicsParams const & igp)
{
    filename = igp.filename;
    display = igp.display;
    inlineFigure = igp.inlineFigure;
    subcaption = igp.subcaption;
    subcaptionText = igp.subcaptionText;
    keepAspectRatio = igp.keepAspectRatio;
    widthResize = igp.widthResize;
    widthSize = igp.widthSize;
    heightResize = igp.heightResize;
    heightSize = igp.heightSize;
    rotateOrigin = igp.rotateOrigin;
    rotateAngle = igp.rotateAngle;

#ifdef ENABLE_ASSERTIONS
    testInvariant();
#endif    
}

void InsetGraphicsParams::testInvariant() const
{
#ifdef ENABLE_ASSERTIONS
    // Filename might be empty (when the dialog is first created).
    // Assert(!filename.empty());
    
    Assert(display == COLOR ||
            display == MONOCHROME ||
            display == GRAYSCALE ||
            display == NONE
            );
    
    Assert(widthResize == DEFAULT_SIZE ||
            widthResize == CM ||
            widthResize == INCH ||
            widthResize == PERCENT_PAGE ||
            widthResize == PERCENT_COLUMN
            );

    Assert(heightResize == DEFAULT_SIZE ||
            heightResize == CM ||
            heightResize == INCH ||
            heightResize == PERCENT_PAGE
            );
    
    Assert(widthSize  >= 0.0);
    Assert(heightSize >= 0.0);
    
    // Angle is in degrees and ranges -360 < angle < 360
    // The reason for this is that in latex there is a meaning for the
    // different angles and they are not necessarliy interchangeable,
    // it depends on the rotation origin.
    Assert(rotateAngle <  360);
    Assert(rotateAngle > -360);

#endif
}

bool operator==(InsetGraphicsParams const & left, 
        InsetGraphicsParams const & right)
{
    if (left.filename == right.filename   &&
            left.display == right.display &&
            left.inlineFigure == right.inlineFigure &&
            left.subcaption == right.subcaption &&
            left.subcaptionText == right.subcaptionText &&
            left.keepAspectRatio == right.keepAspectRatio &&
            left.widthResize == right.widthResize &&
            left.widthSize == right.widthSize &&
            left.heightResize == right.heightResize &&
            left.heightSize == right.heightSize &&
            left.rotateOrigin == right.rotateOrigin &&
            left.rotateAngle == right.rotateAngle
            )
        return true;

    return false;
}

static void writeResize(ostream & os, char const * key, 
        InsetGraphicsParams::Resize resize, double size)
{
    os << ' ' << key << "Resize ";

    os << resizeTranslator.find(resize);
#if 0
    // Old code, before using translators
    switch (resize) {
    case InsetGraphicsParams::DEFAULT_SIZE:
        os << "default";
        break;
        
    case InsetGraphicsParams::CM:
        os << "cm";
        break;
        
    case InsetGraphicsParams::INCH:
        os << "inch";
        break;
        
    case InsetGraphicsParams::PERCENT_PAGE:
        os << "percentOfPage";
        break;
        
    case InsetGraphicsParams::PERCENT_COLUMN:
        os << "percentOfColumnt";
        break;
    }
#endif
    os << ' ' << key << ' ' << size << endl;
}

static void writeOrigin(ostream & os, 
        InsetGraphicsParams::Origin origin)
{
    os << " rotateOrigin " << originTranslator.find(origin);
    
#if 0
    // Old method.
    switch (origin) {
    case InsetGraphicsParams::  DEFAULT:
        os << "default";
        break;
    case InsetGraphicsParams::	LEFTTOP:
        os << "LeftTop";
        break;
    case InsetGraphicsParams::	LEFTCENTER:
        os << "LeftCenter";
        break;
    case InsetGraphicsParams::	LEFTBASELINE:
        os << "LeftBaseLine";
        break;
    case InsetGraphicsParams::	LEFTBOTTOM:
        os << "LeftBottom";
        break;
    case InsetGraphicsParams::	CENTERTOP:
        os << "CenterTop";
        break;
    case InsetGraphicsParams::	CENTER:
        os << "Center";
        break;
    case InsetGraphicsParams::	CENTERBASELINE:
        os << "CenterBaseLine";
        break;
    case InsetGraphicsParams::	CENTERBOTTOM:
        os << "CenterBottom";
        break;
    case InsetGraphicsParams::	RIGHTTOP:
        os << "RightTop";
        break;
    case InsetGraphicsParams::	RIGHTCENTER:
        os << "RightCenter";
        break;
    case InsetGraphicsParams::	RIGHTBASELINE:
        os << "RightBaseLine";
        break;
    case InsetGraphicsParams::	RIGHTBOTTOM:
        os << "RightBottom";
        break;
        // Current REFERENCE_POINT is aliased to LEFTBASELINE
//    case InsetGraphicsParams::	REFERENCE_POINT:
    }
#endif
    
    os << endl;
}

void InsetGraphicsParams::Write(Buffer const * buf, ostream & os) const
{
    // If there is no filename, write nothing for it.
    if (! filename.empty()) {
        os << "filename " 
           << MakeRelPath(filename, OnlyPath(buf->fileName())) 
           << endl;
    }

    // Save the display type
    os << " display " << displayTranslator.find(display) << endl;
#if 0
    switch (display) {
    case COLOR:
        os << "color";
        break;

    case GRAYSCALE:
        os << "grayscale";
        break;

    case MONOCHROME:
        os << "monochrome";
        break;

    case NONE:
        os << "none";
        break;
    }
    os << endl;
#endif
    
    // Save the inline status
    if (inlineFigure)
        os << " inline";

    // Save the subcaption status
    if (subcaption)
        os << " subcaption";

    if (! subcaptionText.empty())
        os << " subcaptionText \"" << subcaptionText << '\"' << endl;

    writeResize(os, "width", widthResize, widthSize);
    writeResize(os, "height", heightResize, heightSize);

    writeOrigin(os, rotateOrigin);
    if (rotateAngle != 0)
        os << " rotateAngle " << rotateAngle << endl;
}
   
static readResize(InsetGraphicsParams * igp, bool height, string const & token)
{
    InsetGraphicsParams::Resize resize = InsetGraphicsParams::DEFAULT_SIZE;

    resize = resizeTranslator.find(token);
#if 0
    // Old code, before translator.
    if (token == "default")
        resize = InsetGraphicsParams::DEFAULT_SIZE;
    else if (token == "cm")
        resize = InsetGraphicsParams::CM;
    else if (token == "inch")
        resize = InsetGraphicsParams::INCH;
    else if (token == "percentOfPage")
        resize = InsetGraphicsParams::PERCENT_PAGE;
    else if (token == "percentOfColumn")
        resize = InsetGraphicsParams::PERCENT_COLUMN;
    else {
        lyxerr << "BUG: When reading resize value of InsetGraphicsParam"
            " unknown token found '" << token << '\'' << endl;
    }
#endif 
    
    if (height)
        igp->heightResize = resize;
    else 
        igp->widthResize = resize;
}
    
static readOrigin(InsetGraphicsParams * igp, string const & token)
{ // TODO: complete this function.
    igp->rotateOrigin = originTranslator.find(token);
}
    
bool InsetGraphicsParams::Read(Buffer const * buf, LyXLex & lex, 
        string const& token)
{
    if (token == "filename") {
        lex.next();
        filename = lex.GetString();

        if (!filename.empty()) {
            // Make the filename with absolute directory.
            filename = MakeAbsPath(filename, OnlyPath(buf->fileName()));
        }
    } else if (token == "display") {
        lex.next();
        string const type = lex.GetString();

        display = displayTranslator.find(type);
#if 0
        if (type == "color")
            display = COLOR;
        else if (type == "grayscale")
            display = GRAYSCALE;
        else if (type == "monochrome")
            display = MONOCHROME;
        else if (type == "none")
            display = NONE;
        else {
            display = MONOCHROME;
            lyxerr << "BUG: When reading InsetGraphicsParams"
                " display has an unknown type " << type << endl;
        }
#endif 
    } else if (token == "inline") {
        inlineFigure = true;
    } else if (token == "subcaption") {
        subcaption = true;
    } else if (token == "subcaptionText") {
        lex.next();
        subcaptionText = lex.GetString();
    } else if (token == "widthResize") {
        lex.next();
        string const token = lex.GetString();
        
        readResize(this, false, token);
    } else if (token == "width") {
        lex.next();
        widthSize = lex.GetFloat();
    } else if (token == "heightResize") {
        lex.next();
        string const token = lex.GetString();
        
        readResize(this, true, token);
    } else if (token == "height") {
        lex.next();
        heightSize = lex.GetFloat();
    } else if (token == "rotateOrigin") {
        lex.next();
        string const token = lex.GetString();

        readOrigin(this, token);
    } else if (token == "rotateAngle") {
        lex.next();
        rotateAngle = lex.GetInteger();
    } else {
        // If it's none of the above, its not ours.
        return false;
    }

    return true;
}
