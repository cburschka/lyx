// -*- C++ -*-
/**
 * \file Compare.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COMPARE_H
#define COMPARE_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>


namespace lyx {

class Buffer;

/**
 * The options that are used by the Comparison algorithm
 * and are set in the GuiCompare Dialog.
 */
class CompareOptions {
public:
	///
	CompareOptions()
		: settings_from_new(0)
	{}

	/// Copy the settings from the new or old document
	bool settings_from_new;
};

/**
 * A threaded object that does the Comparison between two documents
 * and creates a new document with the differences marked with track
 * changes.
 */
class Compare : public QThread
{
	Q_OBJECT

public:
	///
	Compare(Buffer const * const old_buf, Buffer const * const new_buf,
		Buffer * const dest_buf, CompareOptions const & options);

	///
	~Compare() {
		abort();
	}

Q_SIGNALS:
	/// The thread has finished due to an error.
	void error() const;

	/// The thread has finished. If the thread is cancelled
	/// by the user \c aborted is true.
	void finished(bool aborted) const;

	/// Adds \c progress to the value of the progress bar in the dialog
	void progress(int progress) const ;

	/// Sets the maximum value of the progress bar in the dialog.
	void progressMax(int max) const;

	/// A message describing the process
	void statusMessage(QString msg) const;

public Q_SLOTS:
	/// Emits the status message signal
	void doStatusMessage();

public:
	/// \name QThread inherited methods
	//@{
	void run();
	//@}

	/// Aborts the thread
	void abort();

private:
	/// Starts the comparison algorithm
	int doCompare();

	/// The new document's buffer
	Buffer const * const new_buffer;
	/// The old document's buffer
	Buffer const * const old_buffer;
	/// The buffer with the differences marked with track changes
	Buffer * const dest_buffer;

	/// The options that are set in the GuiCompare dialog
	CompareOptions options_;

	///
	QWaitCondition condition_;

	/// Emit a statusMessage signal from time to time
	QTimer status_timer_;

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	///
	Impl * pimpl_;
};


} // namespace lyx

#endif
