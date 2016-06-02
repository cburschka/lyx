// -*- C++ -*-
/**
 * \file ExternalTransforms.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXTERNALTRANSFORMS_H
#define EXTERNALTRANSFORMS_H

#include "Length.h"

#include "graphics/GraphicsParams.h"

#include "support/unique_ptr.h"

#include <boost/any.hpp>
#include <boost/function.hpp>

#include <string>
#include <map>
#include <memory>

namespace lyx {

class Lexer;

namespace external {

/*
 * The data containers
 */
class ClipData {
public:
	ClipData() : clip(false) {}

	/// The bounding box
	graphics::BoundingBox bbox;
	/// clip image
	bool clip;
};


class ExtraData {
public:
	std::string const get(std::string const & id) const;
	void set(std::string const & id, std::string const & contents);

	typedef std::map<std::string, std::string>::const_iterator const_iterator;
	const_iterator begin() const { return data_.begin(); }
	const_iterator end() const { return data_.end(); }

private:
	std::map<std::string, std::string> data_;
};


class ResizeData {
public:
	ResizeData() : scale(), keepAspectRatio(false) {}
	bool no_resize() const;

	bool usingScale() const;

	std::string scale;
	Length width;
	Length height;
	bool keepAspectRatio;
};


class RotationData {
public:
	enum OriginType {
		DEFAULT,
		TOPLEFT,
		BOTTOMLEFT,
		BASELINELEFT,
		CENTER,
		TOPCENTER,
		BOTTOMCENTER,
		BASELINECENTER,
		TOPRIGHT,
		BOTTOMRIGHT,
		BASELINERIGHT
	};

	RotationData() : angle("0"), origin_(DEFAULT) {}
	bool no_rotation() const;

	std::string const adjAngle() const;
	std::string angle;

	void origin(OriginType o) { origin_ = o; }
	OriginType origin() const { return origin_; }

	void origin(std::string const &);
	std::string const originString() const;

private:
	OriginType origin_;
};


/** \c RotationDataType is a wrapper for RotationData::OriginType
 *  It can be forward-declared and passed as a function argument without
 *  having to expose this header file.
 */
class RotationDataType {
	RotationData::OriginType val_;
public:
	RotationDataType(RotationData::OriginType val) : val_(val) {}
	operator RotationData::OriginType() const { return val_; }
};


/*
 * Transformers generating commands
 */
class TransformCommand {
public:
	typedef unique_ptr<TransformCommand const> ptr_type;
	virtual ~TransformCommand() {}

	/// The string from the External Template that we seek to replace.
	std::string const front_placeholder() const
		{ return front_placeholder_impl(); }
	std::string const back_placeholder() const
		{ return back_placeholder_impl(); }

	/// The appropriate replacements for the placeholder strings.
	std::string const front() const { return front_impl(); }
	std::string const back() const { return back_impl(); }

private:
	virtual std::string const front_placeholder_impl() const = 0;
	virtual std::string const back_placeholder_impl() const = 0;

	virtual std::string const front_impl() const = 0;
	virtual std::string const back_impl() const = 0;
};


class ResizeCommand : public TransformCommand {
protected:
	ResizeData data;
	ResizeCommand(ResizeData const & data_) : data(data_) {}

private:
	virtual std::string const front_placeholder_impl() const
		{ return "$$ResizeFront"; }
	virtual std::string const back_placeholder_impl() const
		{ return "$$ResizeBack"; }
};


class ResizeLatexCommand : public ResizeCommand {
public:
	static ptr_type factory(ResizeData const & data)
		{ return ptr_type(new ResizeLatexCommand(data)); }

private:
	ResizeLatexCommand(ResizeData const & data_)
		: ResizeCommand(data_) {}
	virtual std::string const front_impl() const;
	virtual std::string const back_impl() const;
};


class RotationCommand : public TransformCommand {
protected:
	RotationData data;
	RotationCommand(RotationData const & data_) : data(data_) {}

private:
	virtual std::string const front_placeholder_impl() const
		{ return "$$RotateFront"; }
	virtual std::string const back_placeholder_impl() const
		{ return "$$RotateBack"; }
};


class RotationLatexCommand : public RotationCommand {
public:
	static ptr_type factory(RotationData const & data)
		{ return ptr_type(new RotationLatexCommand(data)); }

private:
	RotationLatexCommand(RotationData const & data_)
		: RotationCommand(data_) {}
	virtual std::string const front_impl() const;
	virtual std::string const back_impl() const;
};


/*
 * Transformers generating options
 */
class TransformOption {
public:
	typedef unique_ptr<TransformOption const> ptr_type;
	virtual ~TransformOption() {}

	/// The string from the External Template that we seek to replace.
	std::string const placeholder() const { return placeholder_impl(); }

	/// The appropriate replacement for the placeholder string.
	std::string const option() const { return option_impl(); }

private:
	virtual std::string const placeholder_impl() const = 0;
	virtual std::string const option_impl() const = 0;
};


class ClipOption : public TransformOption {
protected:
	ClipData data;
	ClipOption(ClipData const & data_) : data(data_) {}

private:
	virtual std::string const placeholder_impl() const
		{ return "$$Clip"; }
};


class ClipLatexOption : public ClipOption {
public:
	static ptr_type factory(ClipData const & data)
		{ return ptr_type(new ClipLatexOption(data)); }

private:
	ClipLatexOption(ClipData const & data_)
		: ClipOption(data_) {}
	virtual std::string const option_impl() const;
};


class ExtraOption : public TransformOption {
public:
	static ptr_type factory(std::string const & data)
		{ return ptr_type(new ExtraOption(data)); }

private:
	ExtraOption(std::string const & data_) : data(data_) {}

	virtual std::string const placeholder_impl() const
		{ return "$$Extra"; }
	virtual std::string const option_impl() const
		{ return data; }
	std::string data;
};


class ResizeOption : public TransformOption {
protected:
	ResizeData data;
	ResizeOption(ResizeData const & data_) : data(data_) {}

private:
	virtual std::string const placeholder_impl() const
		{ return "$$Resize"; }
};


class ResizeLatexOption : public ResizeOption {
public:
	static ptr_type factory(ResizeData const & data)
		{ return ptr_type(new ResizeLatexOption(data)); }

private:
	ResizeLatexOption(ResizeData const & data_)
		: ResizeOption(data_) {}
	virtual std::string const option_impl() const;
};


class RotationOption : public TransformOption {
protected:
	RotationData data;
	RotationOption(RotationData const & data_) : data(data_) {}

private:
	virtual std::string const placeholder_impl() const
		{ return "$$Rotate"; }
};


class RotationLatexOption : public RotationOption {
public:
	static ptr_type factory(RotationData const & data)
		{ return ptr_type(new RotationLatexOption(data)); }

private:
	RotationLatexOption(RotationData const & data_)
		: RotationOption(data_) {}
	virtual std::string const option_impl() const;
};


/*
 * Option sanitizers
 */
std::string const sanitizeLatexOption(std::string const & input);
std::string const sanitizeDocBookOption(std::string const & input);


enum TransformID {
	Rotate,
	Resize,
	Clip,
	Extra
};


typedef boost::function<TransformOption::ptr_type(ClipData)>
	ClipOptionFactory;
typedef boost::function<TransformOption::ptr_type(std::string)>
	ExtraOptionFactory;
typedef boost::function<TransformOption::ptr_type(ResizeData)>
	ResizeOptionFactory;
typedef boost::function<TransformOption::ptr_type(RotationData)>
	RotationOptionFactory;
typedef boost::function<TransformCommand::ptr_type(ResizeData)>
	ResizeCommandFactory;
typedef boost::function<TransformCommand::ptr_type(RotationData)>
	RotationCommandFactory;


class TransformStore
{
public:
	TransformStore() {}

	/** Stores \c factory and a reminder of what \c data this \c factory
	 *  operates on.
	 */
	template <typename Factory>
	TransformStore(TransformID id_, Factory const & factory)
		: id(id_), any_factory(boost::any(factory)) {}

	typedef TransformCommand::ptr_type ComPtr;
	typedef TransformOption::ptr_type  OptPtr;

	ComPtr getCommandTransformer(RotationData const &) const;
	ComPtr getCommandTransformer(ResizeData const &) const;
	OptPtr getOptionTransformer(RotationData const &) const;
	OptPtr getOptionTransformer(ResizeData const &) const;
	OptPtr getOptionTransformer(ClipData const &) const;
	OptPtr getOptionTransformer(std::string const &) const;

private:
	TransformID id;
	boost::any any_factory;
};

} // namespace external
} // namespace lyx

#endif // NOT EXTERNALTRANSFORMS_H
