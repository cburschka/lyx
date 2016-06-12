/**
 * \file ExternalTransforms.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ExternalTransforms.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lstrings.h"
#include "support/lyxlib.h" // float_equal
#include "support/Translator.h"

#include "support/regex.h"

#include <cmath> // abs
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace external {


string const ExtraData::get(string const & id) const
{
	map<string, string>::const_iterator it = data_.find(id);
	return it == data_.end() ? string() : it->second;
}


void ExtraData::set(string const & id, string const & data)
{
	data_[id] = data;
}


bool ResizeData::no_resize() const
{
	return !usingScale() && width.zero() && height.zero();
}


bool ResizeData::usingScale() const
{
	return (!scale.empty() && !float_equal(convert<double>(scale), 0.0, 0.05));
}


bool RotationData::no_rotation() const
{
	return (angle.empty() || abs(convert<double>(angle)) < 0.1);
}


string const RotationData::adjAngle() const
{
	// Ensure that angle lies in the range -360 < angle < 360
	double rotAngle = convert<double>(angle);
	if (abs(rotAngle) > 360.0) {
		rotAngle -= 360.0 * floor(rotAngle / 360.0);
		return convert<string>(rotAngle);
	}
	return angle;
}


namespace {

typedef Translator<RotationData::OriginType, string> OriginTranslator;
OriginTranslator const & originTranslator();

} // namespace anon


void RotationData::origin(string const & o)
{
	origin_ = originTranslator().find(o);
}


string const RotationData::originString() const
{
	return originTranslator().find(origin_);
}


string const ResizeLatexCommand::front_impl() const
{
	if (data.no_resize())
		return string();

	ostringstream os;
	if (data.usingScale()) {
		double const scl = convert<double>(data.scale) / 100.0;
		os << "\\scalebox{" << scl << "}[" << scl << "]{";
	} else {
		string width  = "!";
		string height = "!";
		if (data.keepAspectRatio) {
			if (data.width.inPixels(10) > data.height.inPixels(10))
				width = data.width.asLatexString();
			else
				height = data.height.asLatexString();
		} else {
			if (!data.width.zero())
				width = data.width.asLatexString();
			if (!data.height.zero())
				height = data.height.asLatexString();
		}

		os << "\\resizebox{"
		   << width << "}{"
		   << height << "}{";
	}
	return os.str();
}


string const ResizeLatexCommand::back_impl() const
{
	if (data.no_resize())
		return string();

	return "}";
}


namespace {

ostream & operator<<(ostream & os, RotationData::OriginType type)
{
	switch (type) {
	case RotationData::DEFAULT:
	case RotationData::CENTER:
		break;
	case RotationData::TOPLEFT:
	case RotationData::TOPCENTER:
	case RotationData::TOPRIGHT:
		os << 't';
		break;
	case RotationData::BOTTOMLEFT:
	case RotationData::BOTTOMCENTER:
	case RotationData::BOTTOMRIGHT:
		os << 'b';
		break;
	case RotationData::BASELINELEFT:
	case RotationData::BASELINECENTER:
	case RotationData::BASELINERIGHT:
		os << 'B';
		break;
	}

	switch (type) {
	case RotationData::DEFAULT:
		break;
	case RotationData::TOPLEFT:
	case RotationData::BOTTOMLEFT:
	case RotationData::BASELINELEFT:
		os << 'l';
		break;
	case RotationData::CENTER:
	case RotationData::TOPCENTER:
	case RotationData::BOTTOMCENTER:
	case RotationData::BASELINECENTER:
		os << 'c';
		break;
	case RotationData::TOPRIGHT:
	case RotationData::BOTTOMRIGHT:
	case RotationData::BASELINERIGHT:
		os << 'r';
		break;
	}

	return os;
}

} // namespace anon


string const RotationLatexCommand::front_impl() const
{
	if (data.no_rotation())
		return string();

	ostringstream os;
	os << "\\rotatebox";

	if (data.origin() != RotationData::DEFAULT)
		os << "[origin=" << data.origin() << ']';

	os << '{' << data.angle << "}{";
	return os.str();
}


string const RotationLatexCommand::back_impl() const
{
	if (data.no_rotation())
		return string();

	return "}";
}


string const  ClipLatexOption::option_impl() const
{
	if (!data.clip || data.bbox.empty())
		return string();

	ostringstream os;
	if (!data.bbox.empty())
		os << "bb=" << data.bbox.xl.asLatexString() << ' '
		   << data.bbox.yb.asLatexString() << ' '
		   << data.bbox.xr.asLatexString() << ' '
		   << data.bbox.yt.asLatexString() << ',';
	if (data.clip)
		os << "clip,";
	return os.str();
}


string const ResizeLatexOption::option_impl() const
{
	if (data.no_resize())
		return string();

	ostringstream os;
	if (data.usingScale()) {
		double const scl = convert<double>(data.scale);
		if (!float_equal(scl, 100.0, 0.05))
			os << "scale=" << scl / 100.0 << ',';
		return os.str();
	}

	if (!data.width.zero())
		os << "width=" << data.width.asLatexString() << ',';
	if (!data.height.zero())
		os << "height=" << data.height.asLatexString() << ',';
	if (data.keepAspectRatio)
		os << "keepaspectratio,";

	return os.str();
}


string const RotationLatexOption ::option_impl() const
{
	if (data.no_rotation())
		return string();

	ostringstream os;
	os << "angle=" << data.angle << ',';

	if (data.origin() != RotationData::DEFAULT)
		os << "origin=" << data.origin() << ',';

	return os.str();
}


string const sanitizeDocBookOption(string const & input)
{
	return input;
}


string const sanitizeLatexOption(string const & input)
{
	string::const_iterator begin = input.begin();
	string::const_iterator end   = input.end();
	string::const_iterator it = begin;

	// Strip any leading commas
	// "[,,,,foo..." -> "foo..." ("foo..." may be empty)
	string output;
	lyx::smatch what;
	static lyx::regex const front("^( *\\[,*)(.*)$");

	regex_match(it, end, what, front);
	if (!what[0].matched) {
		lyxerr << "Unable to sanitize LaTeX \"Option\": "
		       << input << '\n';
		return string();
	}
	it =  what[1].second;

	// Replace any consecutive commas with a single one
	// "foo,,,,bar" -> "foo,bar"
	// with iterator now pointing to 'b'
	static lyx::regex const commas("([^,]*)(,,*)(.*)$");
	for (; it != end;) {
		regex_match(it, end, what, commas);
		if (!what[0].matched) {
			output += string(it, end);
			break;
		}
		output += what.str(1) + ",";
		it = what[3].first;
	}

	// Strip any trailing commas
	// "...foo,,,]" -> "...foo" ("...foo,,," may be empty)
	static lyx::regex const back("^(.*[^,])?,*\\] *$");
	// false positive from coverity
	// coverity[CHECKED_RETURN]
	regex_match(output, what, back);
	if (!what[0].matched) {
		lyxerr << "Unable to sanitize LaTeX \"Option\": "
		       << output << '\n';
		return string();
	}
	output = what.str(1);

	// Remove any surrounding whitespace
	output = trim(output);

	// If the thing is empty, leave it so, else wrap it in square brackets.
	return output.empty() ? output : "[" + output + "]";
}


namespace {

template <typename Factory, typename Data, typename Transformer>
void extractIt(boost::any const & any_factory,
	       Data const & data, Transformer & transformer)
{
	if (any_factory.type() != typeid(Factory))
		return;

	Factory factory = boost::any_cast<Factory>(any_factory);
	if (!factory.empty())
		transformer = factory(data);
}

} // namespace anon


TransformCommand::ptr_type
TransformStore::getCommandTransformer(RotationData const & data) const
{
	TransformCommand::ptr_type ptr;
	if (id == Rotate)
		extractIt<RotationCommandFactory>(any_factory, data, ptr);
	return ptr;
}


TransformCommand::ptr_type
TransformStore::getCommandTransformer(ResizeData const & data) const
{
	TransformCommand::ptr_type ptr;
	if (id == Resize)
		extractIt<ResizeCommandFactory>(any_factory, data, ptr);
	return ptr;
}


TransformOption::ptr_type
TransformStore::getOptionTransformer(RotationData const & data) const
{
	TransformOption::ptr_type ptr;
	if (id == Rotate)
		extractIt<RotationOptionFactory>(any_factory, data, ptr);
	return ptr;
}


TransformOption::ptr_type
TransformStore::getOptionTransformer(ResizeData const & data) const
{
	TransformOption::ptr_type ptr;
	if (id == Resize)
		extractIt<ResizeOptionFactory>(any_factory, data, ptr);
	return ptr;
}


TransformOption::ptr_type
TransformStore::getOptionTransformer(ClipData const & data) const
{
	TransformOption::ptr_type ptr;
	if (id == Clip)
		extractIt<ClipOptionFactory>(any_factory, data, ptr);
	return ptr;
}



TransformOption::ptr_type
TransformStore::getOptionTransformer(string const & data) const
{
	TransformOption::ptr_type ptr;
	if (id == Extra)
		extractIt<ExtraOptionFactory>(any_factory, data, ptr);
	return ptr;
}


namespace {

OriginTranslator const initOriginTranslator()
{
	OriginTranslator translator(RotationData::DEFAULT, "default");
	translator.addPair(RotationData::TOPLEFT,        "topleft");
	translator.addPair(RotationData::BOTTOMLEFT,     "bottomleft");
	translator.addPair(RotationData::BASELINELEFT,   "baselineleft");
	translator.addPair(RotationData::CENTER,         "center");
	translator.addPair(RotationData::TOPCENTER,      "topcenter");
	translator.addPair(RotationData::BOTTOMCENTER,   "bottomcenter");
	translator.addPair(RotationData::BASELINECENTER, "baselinecenter");
	translator.addPair(RotationData::TOPRIGHT,       "topright");
	translator.addPair(RotationData::BOTTOMRIGHT,    "bottomright");
	translator.addPair(RotationData::BASELINERIGHT,  "baselineright");
	return translator;
}


OriginTranslator const & originTranslator()
{
	static OriginTranslator const translator = initOriginTranslator();
	return translator;
}

} // namespace anon

} // namespace external
} // namespace lyx
