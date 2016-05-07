// -*- C++ -*-
/**
 * \file qt4/GuiClipboard.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FileDialog.h"

#include "support/FileName.h"
#include "GuiClipboard.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxtime.h"

#ifdef Q_OS_MAC
#include "support/linkback/LinkBackProxy.h"
#endif // Q_OS_MAC

#include "frontends/alert.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDataStream>
#include <QFile>
#include <QImage>
#include <QMimeData>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QTimer>

#include <boost/crc.hpp>

#include <memory>
#include <map>
#include <iostream>

using namespace std;
using namespace lyx::support;


namespace lyx {

namespace frontend {

static QMimeData const * read_clipboard()
{
	LYXERR(Debug::CLIPBOARD, "Getting Clipboard");
	QMimeData const * source =
		qApp->clipboard()->mimeData(QClipboard::Clipboard);
	if (!source) {
		LYXERR0("0 bytes (no QMimeData)");
		return new QMimeData;
	}
	// It appears that doing IO between getting a mimeData object
	// and using it can cause a crash (maybe Qt used IO
	// as an excuse to free() it? Anyway let's not introduce
	// any new IO here, so e.g. leave the following line commented.
	// lyxerr << "Got Clipboard (" << (long) source << ")\n" ;
	return source;
}


void CacheMimeData::update()
{
	time_t const start_time = current_time();
	LYXERR(Debug::CLIPBOARD, "Creating CacheMimeData object");
	cached_formats_ = read_clipboard()->formats();

	// Qt times out after 5 seconds if it does not recieve a response.
	if (current_time() - start_time > 3) {
		LYXERR0("No timely response from clipboard, perhaps process "
			<< "holding clipboard is frozen?");
	}
}


QByteArray CacheMimeData::data(QString const & mimeType) const
{
	return read_clipboard()->data(mimeType);
}


QString const lyxMimeType(){ return "application/x-lyx"; }
QString const texMimeType(){ return "text/x-tex"; }
QString const latexMimeType(){ return "application/x-latex"; }
QString const pdfMimeType(){ return "application/pdf"; }
QString const emfMimeType(){ return "image/x-emf"; }
QString const wmfMimeType(){ return "image/x-wmf"; }


GuiClipboard::GuiClipboard()
{
	connect(qApp->clipboard(), SIGNAL(dataChanged()),
		this, SLOT(on_dataChanged()));
	// initialize clipboard status.
	update();
}


string const GuiClipboard::getAsLyX() const
{
	LYXERR(Debug::CLIPBOARD, "GuiClipboard::getAsLyX(): `");
	// We don't convert encodings here since the encoding of the
	// clipboard contents is specified in the data itself
	if (cache_.hasFormat(lyxMimeType())) {
		// data from ourself or some other LyX instance
		QByteArray const ar = cache_.data(lyxMimeType());
		string const s(ar.data(), ar.count());
		LYXERR(Debug::CLIPBOARD, s << "'");
		return s;
	}
	LYXERR(Debug::CLIPBOARD, "'");
	return string();
}


FileName GuiClipboard::getPastedGraphicsFileName(Cursor const & cur,
	Clipboard::GraphicsType & type) const
{
	// create file dialog filter according to the existing types in the clipboard
	vector<Clipboard::GraphicsType> types;
	if (hasGraphicsContents(Clipboard::EmfGraphicsType))
		types.push_back(Clipboard::EmfGraphicsType);
	if (hasGraphicsContents(Clipboard::WmfGraphicsType))
		types.push_back(Clipboard::WmfGraphicsType);
	if (hasGraphicsContents(Clipboard::LinkBackGraphicsType))
		types.push_back(Clipboard::LinkBackGraphicsType);
	if (hasGraphicsContents(Clipboard::PdfGraphicsType))
		types.push_back(Clipboard::PdfGraphicsType);
	if (hasGraphicsContents(Clipboard::PngGraphicsType))
		types.push_back(Clipboard::PngGraphicsType);
	if (hasGraphicsContents(Clipboard::JpegGraphicsType))
		types.push_back(Clipboard::JpegGraphicsType);

	LASSERT(!types.empty(), return FileName());

	// select prefered type if AnyGraphicsType was passed
	if (type == Clipboard::AnyGraphicsType)
		type = types.front();

	// which extension?
	map<Clipboard::GraphicsType, string> extensions;
	map<Clipboard::GraphicsType, docstring> typeNames;

	extensions[Clipboard::EmfGraphicsType] = "emf";
	extensions[Clipboard::WmfGraphicsType] = "wmf";
	extensions[Clipboard::LinkBackGraphicsType] = "linkback";
	extensions[Clipboard::PdfGraphicsType] = "pdf";
	extensions[Clipboard::PngGraphicsType] = "png";
	extensions[Clipboard::JpegGraphicsType] = "jpeg";

	typeNames[Clipboard::EmfGraphicsType] = _("Enhanced Metafile");
	typeNames[Clipboard::WmfGraphicsType] = _("Windows Metafile");
	typeNames[Clipboard::LinkBackGraphicsType] = _("LinkBack PDF");
	typeNames[Clipboard::PdfGraphicsType] = _("PDF");
	typeNames[Clipboard::PngGraphicsType] = _("PNG");
	typeNames[Clipboard::JpegGraphicsType] = _("JPEG");

	// find unused filename with primary extension
	string document_path = cur.buffer()->fileName().onlyPath().absFileName();
	unsigned newfile_number = 0;
	FileName filename;
	do {
		++newfile_number;
		filename = FileName(addName(document_path,
			to_utf8(_("pasted"))
			+ convert<string>(newfile_number) + "."
			+ extensions[type]));
	} while (filename.isReadableFile());

	while (true) {
		// create file type filter, putting the prefered on to the front
		QStringList filter;
		for (size_t i = 0; i != types.size(); ++i) {
			docstring s = bformat(_("%1$s Files"), typeNames[types[i]])
				+ " (*." + from_ascii(extensions[types[i]]) + ")";
			if (types[i] == type)
				filter.prepend(toqstr(s));
			else
				filter.append(toqstr(s));
		}
		filter = fileFilters(filter.join(";;"));

		// show save dialog for the graphic
		FileDialog dlg(qt_("Choose a filename to save the pasted graphic as"));
		FileDialog::Result result =
		dlg.save(toqstr(filename.onlyPath().absFileName()), filter,
			 toqstr(filename.onlyFileName()));

		if (result.first == FileDialog::Later)
			return FileName();

		string newFilename = fromqstr(result.second);
		if (newFilename.empty()) {
			cur.bv().message(_("Canceled."));
			return FileName();
		}
		filename.set(newFilename);

		// check the extension (the user could have changed it)
		if (!suffixIs(ascii_lowercase(filename.absFileName()),
			      "." + extensions[type])) {
			// the user changed the extension. Check if the type is available
			size_t i;
			for (i = 1; i != types.size(); ++i) {
				if (suffixIs(ascii_lowercase(filename.absFileName()),
					     "." + extensions[types[i]])) {
					type = types[i];
					break;
				}
			}

			// invalid extension found, or none at all. In the latter
			// case set the default extensions.
			if (i == types.size()
			    && filename.onlyFileName().find('.') == string::npos) {
				filename.changeExtension("." + extensions[type]);
			}
		}

		// check whether the file exists and warn the user
		if (!filename.exists())
			break;
		int ret = frontend::Alert::prompt(
			_("Overwrite external file?"),
			bformat(_("File %1$s already exists, do you want to overwrite it?"),
			from_utf8(filename.absFileName())), 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret == 0)
			// overwrite, hence break the dialog loop
			break;

		// not overwrite, hence show the dialog again (i.e. loop)
	}

	return filename;
}


FileName GuiClipboard::getAsGraphics(Cursor const & cur, GraphicsType type) const
{
	// get the filename from the user
	FileName filename = getPastedGraphicsFileName(cur, type);
	if (filename.empty())
		return FileName();

	// handle image cases first
	if (type == PngGraphicsType || type == JpegGraphicsType) {
		// get image from QImage from clipboard
		QImage image = qApp->clipboard()->image();
		if (image.isNull()) {
			LYXERR(Debug::CLIPBOARD, "No image in clipboard");
			return FileName();
		}

		// convert into graphics format
		QByteArray ar;
		QBuffer buffer(&ar);
		buffer.open(QIODevice::WriteOnly);
		if (type == PngGraphicsType)
			image.save(toqstr(filename.absFileName()), "PNG");
		else if (type == JpegGraphicsType)
			image.save(toqstr(filename.absFileName()), "JPEG");
		else
			LATTEST(false);

		return filename;
	}

	// get mime for type
	QString mime;
	switch (type) {
	case PdfGraphicsType: mime = pdfMimeType(); break;
	case LinkBackGraphicsType: mime = pdfMimeType(); break;
	case EmfGraphicsType: mime = emfMimeType(); break;
	case WmfGraphicsType: mime = wmfMimeType(); break;
	default: LASSERT(false, return FileName());
	}

	// get data
	if (!cache_.hasFormat(mime))
		return FileName();
	// data from ourself or some other LyX instance
	QByteArray const ar = cache_.data(mime);
	LYXERR(Debug::CLIPBOARD, "Getting from clipboard: mime = " << mime.constData()
	       << "length = " << ar.count());

	QFile f(toqstr(filename.absFileName()));
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		LYXERR(Debug::CLIPBOARD, "Error opening file "
		       << filename.absFileName() << " for writing");
		return FileName();
	}

	// write the (LinkBack) PDF data
	f.write(ar);
	if (type == LinkBackGraphicsType) {
#ifdef Q_OS_MAC
		void const * linkBackData;
		unsigned linkBackLen;
		getLinkBackData(&linkBackData, &linkBackLen);
		f.write((char *)linkBackData, linkBackLen);
		quint32 pdfLen = ar.size();
		QDataStream ds(&f);
		ds << pdfLen; // big endian by default
#else
		// only non-Mac this should never happen
		LATTEST(false);
#endif // Q_OS_MAC
	}

	f.close();
	return filename;
}


namespace {
/**
 * Tidy up a HTML chunk coming from the clipboard.
 * This is needed since different applications put different kinds of HTML
 * on the clipboard:
 * - With or without the <?xml> tag
 * - With or without the <!DOCTYPE> tag
 * - With or without the <html> tag
 * - With or without the <body> tag
 * - With or without the <p> tag
 * Since we are going to write a HTML file for external converters we need
 * to ensure that it is a well formed HTML file, including all the mentioned tags.
 */
QString tidyHtml(QString input)
{
	// Misuse QTextDocument to cleanup the HTML.
	// As a side effect, all visual markup like <tt> is converted to CSS,
	// which is ignored by gnuhtml2latex.
	// While this may be seen as a bug by some people it is actually a
	// good thing, since we do import structure, but ignore all visual
	// clutter.
	QTextDocument converter;
	converter.setHtml(input);
	return converter.toHtml("utf-8");
}
}


docstring const GuiClipboard::getAsText(TextType type) const
{
	// text data from other applications
	if ((type == AnyTextType || type == LyXOrPlainTextType) && hasTextContents(LyXTextType))
		type = LyXTextType;
	if (type == AnyTextType && hasTextContents(LaTeXTextType))
		type = LaTeXTextType;
	if (type == AnyTextType && hasTextContents(HtmlTextType))
		type = HtmlTextType;
	QString str;
	switch (type) {
	case LyXTextType:
		// must not convert to docstring, since file can contain
		// mixed encodings (use getAsLyX() instead)
		break;
	case AnyTextType:
	case LyXOrPlainTextType:
	case PlainTextType:
		str = qApp->clipboard()->text(QClipboard::Clipboard)
				.normalized(QString::NormalizationForm_C);
		break;
	case LaTeXTextType: {
		QMimeData const * source =
			qApp->clipboard()->mimeData(QClipboard::Clipboard);
		if (source) {
			// First try LaTeX, then TeX (we do not distinguish
			// for clipboard purposes)
			if (source->hasFormat(latexMimeType())) {
				str = source->data(latexMimeType());
				str = str.normalized(QString::NormalizationForm_C);
			} else if (source->hasFormat(texMimeType())) {
				str = source->data(texMimeType());
				str = str.normalized(QString::NormalizationForm_C);
			}
		}
		break;
	}
	case HtmlTextType: {
		QString subtype = "html";
		str = qApp->clipboard()->text(subtype, QClipboard::Clipboard)
				.normalized(QString::NormalizationForm_C);
		str = tidyHtml(str);
		break;
	}
	}
	LYXERR(Debug::CLIPBOARD, "GuiClipboard::getAsText(" << type << "): `" << str << "'");
	if (str.isNull())
		return docstring();

	return internalLineEnding(str);
}


void GuiClipboard::put(string const & text) const
{
	qApp->clipboard()->setText(toqstr(text));
}


void GuiClipboard::put(string const & lyx, docstring const & html, docstring const & text)
{
	LYXERR(Debug::CLIPBOARD, "GuiClipboard::put(`" << lyx << "' `"
			      << to_utf8(html) << "' `" << to_utf8(text) << "')");
	// We don't convert the encoding of lyx since the encoding of the
	// clipboard contents is specified in the data itself
	QMimeData * data = new QMimeData;
	if (!lyx.empty()) {
		QByteArray const qlyx(lyx.c_str(), lyx.size());
		data->setData(lyxMimeType(), qlyx);
		// If the OS has not the concept of clipboard ownership,
		// we recognize internal data through its checksum.
		if (!hasInternal()) {
			boost::crc_32_type crc32;
			crc32.process_bytes(lyx.c_str(), lyx.size());
			checksum = crc32.checksum();
		}
	}
	// Don't test for text.empty() since we want to be able to clear the
	// clipboard.
	QString const qtext = toqstr(text);
	data->setText(qtext);
	QString const qhtml = toqstr(html);
	data->setHtml(qhtml);
	qApp->clipboard()->setMimeData(data, QClipboard::Clipboard);
}


bool GuiClipboard::hasTextContents(Clipboard::TextType type) const
{
	switch (type) {
	case AnyTextType:
		return cache_.hasFormat(lyxMimeType()) || cache_.hasText() ||
		       cache_.hasHtml() || cache_.hasFormat(latexMimeType()) ||
		       cache_.hasFormat(texMimeType());
	case LyXOrPlainTextType:
		return cache_.hasFormat(lyxMimeType()) || cache_.hasText();
	case LyXTextType:
		return cache_.hasFormat(lyxMimeType());
	case PlainTextType:
		return cache_.hasText();
	case HtmlTextType:
		return cache_.hasHtml();
	case LaTeXTextType:
		return cache_.hasFormat(latexMimeType()) ||
		       cache_.hasFormat(texMimeType());
	}
	// shut up compiler
	return false;
}


bool GuiClipboard::hasGraphicsContents(Clipboard::GraphicsType type) const
{
	if (type == AnyGraphicsType) {
		return hasGraphicsContents(PdfGraphicsType)
			|| hasGraphicsContents(PngGraphicsType)
			|| hasGraphicsContents(JpegGraphicsType)
			|| hasGraphicsContents(EmfGraphicsType)
			|| hasGraphicsContents(WmfGraphicsType)
			|| hasGraphicsContents(LinkBackGraphicsType);
	}

	// handle image cases first
	if (type == PngGraphicsType || type == JpegGraphicsType)
		return cache_.hasImage();

	// handle LinkBack for Mac
	if (type == LinkBackGraphicsType)
#ifdef Q_OS_MAC
		return isLinkBackDataInPasteboard();
#else
		return false;
#endif // Q_OS_MAC

	// get mime data
	QStringList const & formats = cache_.formats();
	LYXERR(Debug::CLIPBOARD, "We found " << formats.size() << " formats");
	for (int i = 0; i < formats.size(); ++i)
		LYXERR(Debug::CLIPBOARD, "Found format " << formats[i]);

	// compute mime for type
	QString mime;
	switch (type) {
	case EmfGraphicsType: mime = emfMimeType(); break;
	case WmfGraphicsType: mime = wmfMimeType(); break;
	case PdfGraphicsType: mime = pdfMimeType(); break;
	default: LASSERT(false, return false);
	}

	return cache_.hasFormat(mime);
}


bool GuiClipboard::isInternal() const
{
	if (!hasTextContents(LyXTextType))
		return false;

	// ownsClipboard() is also true for stuff coming from dialogs, e.g.
	// the preamble dialog. This does only work on X11 and Windows, since
	// ownsClipboard() is hardwired to return false on OS X.
	if (hasInternal())
		return qApp->clipboard()->ownsClipboard();

	// We are running on OS X: Check whether clipboard data is from
	// ourself by comparing its checksum with the stored one.
	QByteArray const ar = cache_.data(lyxMimeType());
	string const data(ar.data(), ar.count());
	boost::crc_32_type crc32;
	crc32.process_bytes(data.c_str(), data.size());
	return checksum == crc32.checksum();
}


bool GuiClipboard::hasInternal() const
{
	// Windows and Mac OS X does not have the concept of ownership;
	// the clipboard is a fully global resource so all applications
	// are notified of changes. However, on Windows ownership is
	// emulated by Qt through the OleIsCurrentClipboard() API, while
	// on Mac OS X we deal with this issue by ourself.
#ifndef Q_OS_MAC
	return true;
#else
	return false;
#endif
}


void GuiClipboard::on_dataChanged()
{
	update();
#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
	// Retry on Windows (#10109)
	if (cache_.formats().count() == 0) {
		QTimer::singleShot(100, this, SLOT(update()));
	}
#endif
}

void GuiClipboard::update()
{
	//Note: we do not really need to run cache_.update() unless the
	//data has been changed *and* the GuiClipboard has been queried.
	//However if run cache_.update() the moment a process grabs the
	//clipboard, the process holding the clipboard presumably won't
	//yet be frozen, and so we won't need to wait 5 seconds for Qt
	//to time-out waiting for the clipboard.
	cache_.update();
	QStringList l = cache_.formats();
	LYXERR(Debug::CLIPBOARD, "Qt Clipboard changed. We found the following mime types:");
	for (int i = 0; i < l.count(); i++)
		LYXERR(Debug::CLIPBOARD, l.value(i));

	plaintext_clipboard_empty_ = qApp->clipboard()->
		text(QClipboard::Clipboard).isEmpty();

	has_text_contents_ = hasTextContents();
	has_graphics_contents_ = hasGraphicsContents();
}


bool GuiClipboard::empty() const
{
	// We need to check both the plaintext and the LyX version of the
	// clipboard. The plaintext version is empty if the LyX version
	// contains only one inset, and the LyX version is empty if the
	// clipboard does not come from LyX.
	if (!plaintext_clipboard_empty_)
		return false;
	return !has_text_contents_ && !has_graphics_contents_;
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiClipboard.cpp"
