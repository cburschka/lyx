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

#include "graphics/GraphicsParams.h"

#include "support/any.h"
#include "support/Length.h"
#include "support/unique_ptr.h"

#include <functional>
#include <map>
#include <memory>
#include <string>


namespace lyx {

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
	void set(std::string const & id, std::string const & data);

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
	explicit ResizeCommand(ResizeData const & data_) : data(data_) {}

private:
	std::string const front_placeholder_impl() const override
		{ return "$$ResizeFront"; }
	std::string const back_placeholder_impl() const override
		{ return "$$ResizeBack"; }
};


class ResizeLatexCommand : public ResizeCommand {
public:
	static ptr_type factory(ResizeData const & data)
		{ return ptr_type(new ResizeLatexCommand(data)); }

private:
	explicit ResizeLatexCommand(ResizeData const & data_)
		: ResizeCommand(data_) {}
	std::string const front_impl() const override;
	std::string const back_impl() const override;
};


class RotationCommand : public TransformCommand {
protected:
	RotationData data;
	explicit RotationCommand(RotationData const & data_) : data(data_) {}

private:
	std::string const front_placeholder_impl() const override
		{ return "$$RotateFront"; }
	std::string const back_placeholder_impl() const override
		{ return "$$RotateBack"; }
};


class RotationLatexCommand : public RotationCommand {
public:
	static ptr_type factory(RotationData const & data)
		{ return ptr_type(new RotationLatexCommand(data)); }

private:
	explicit RotationLatexCommand(RotationData const & data_)
		: RotationCommand(data_) {}
	std::string const front_impl() const override;
	std::string const back_impl() const override;
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
	explicit ClipOption(ClipData const & data_) : data(data_) {}

private:
	std::string const placeholder_impl() const override
		{ return "$$Clip"; }
};


class ClipLatexOption : public ClipOption {
public:
	static ptr_type factory(ClipData const & data)
		{ return ptr_type(new ClipLatexOption(data)); }

private:
	explicit ClipLatexOption(ClipData const & data_)
		: ClipOption(data_) {}
	std::string const option_impl() const override;
};


class ExtraOption : public TransformOption {
public:
	static ptr_type factory(std::string const & data)
		{ return ptr_type(new ExtraOption(data)); }

private:
	explicit ExtraOption(std::string const & data_) : data(data_) {}

	std::string const placeholder_impl() const override
		{ return "$$Extra"; }
	std::string const option_impl() const override
		{ return data; }
	std::string data;
};


class ResizeOption : public TransformOption {
protected:
	ResizeData data;
	explicit ResizeOption(ResizeData const & data_) : data(data_) {}

private:
	std::string const placeholder_impl() const override
		{ return "$$Resize"; }
};


class ResizeLatexOption : public ResizeOption {
public:
	static ptr_type factory(ResizeData const & data)
		{ return ptr_type(new ResizeLatexOption(data)); }

private:
	explicit ResizeLatexOption(ResizeData const & data_)
		: ResizeOption(data_) {}
	std::string const option_impl() const override;
};


class RotationOption : public TransformOption {
protected:
	RotationData data;
	explicit RotationOption(RotationData const & data_) : data(data_) {}

private:
	std::string const placeholder_impl() const override
		{ return "$$Rotate"; }
};


class RotationLatexOption : public RotationOption {
public:
	static ptr_type factory(RotationData const & data)
		{ return ptr_type(new RotationLatexOption(data)); }

private:
	explicit RotationLatexOption(RotationData const & data_)
		: RotationOption(data_) {}
	std::string const option_impl() const override;
};


/*
 * Option sanitizers
 */
std::string const sanitizeLatexOption(std::string const & input);
std::string const sanitizeDocBookOption(std::string const & input);


enum TransformID {
	None = -1,
	Rotate,
	Resize,
	Clip,
	Extra
};


typedef std::function<TransformOption::ptr_type(ClipData)>
	ClipOptionFactory;
typedef std::function<TransformOption::ptr_type(std::string)>
	ExtraOptionFactory;
typedef std::function<TransformOption::ptr_type(ResizeData)>
	ResizeOptionFactory;
typedef std::function<TransformOption::ptr_type(RotationData)>
	RotationOptionFactory;
typedef std::function<TransformCommand::ptr_type(ResizeData)>
	ResizeCommandFactory;
typedef std::function<TransformCommand::ptr_type(RotationData)>
	RotationCommandFactory;


class TransformStore
{
public:
	TransformStore() : id(Rotate) {}

	/** Stores \c factory and a reminder of what \c data this \c factory
	 *  operates on.
	 */
	template <typename Factory>
	TransformStore(TransformID id_, Factory const & factory)
		: id(id_), any_factory(any(factory)) {}

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
	any any_factory;
};

} // namespace external
} // namespace lyx

#endif // NOT EXTERNALTRANSFORMS_H
