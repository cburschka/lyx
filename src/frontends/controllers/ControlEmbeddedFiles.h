// -*- C++ -*-
/**
 * \file ControlEmbeddedFiles.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLEMBEDDEDFILES_H
#define CONTROLEMBEDDEDFILES_H

#include "Dialog.h"
#include "EmbeddedFiles.h"

namespace lyx {

namespace frontend {

class ControlEmbeddedFiles : public Controller {
public:
	///
	ControlEmbeddedFiles(Dialog &);
	///
	virtual ~ControlEmbeddedFiles() {}
	///
	EmbeddedFiles & embeddedFiles();
	///
	virtual bool initialiseParams(std::string const &);
	/// obtain embedded files from buffer
	void updateEmbeddedFiles();
	///
	virtual void clearParams() {};
	///
	virtual bool isBufferDependent() const { return true; }
	///
	bool canApply() const { return true; }
	///
	virtual bool canApplyToReadOnly() const { return true; }
	///
	void dispatchMessage(std::string const & msg);
	///
	void dispatchParams() {};
	///
	void setEmbedding(bool enable);
	///
	void goTo(EmbeddedFile const & item, int idx);
	///
	void view(EmbeddedFile const & item);
	///
	void setEmbed(EmbeddedFile & item, bool embed, bool update);
	///
	bool browseAndAddFile();
	///
	bool extract(EmbeddedFile const & item);
	///
	bool update(EmbeddedFile const & item);

protected:
	//
	std::string message_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLEMBEDDEDFILES_H
