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

#ifdef Q_WS_MACX
#include "support/linkback/LinkBackProxy.h"
#endif // Q_WS_MACX

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

#include <memory>
#include <map>

using namespace std;
using namespace lyx::support;

static char const * const lyx_mime_type = "application/x-lyx";
static char const * const pdf_mime_type = "application/pdf";
static char const * const emf_mime_type = "image/x-emf";
static char const * const wmf_mime_type = "image/x-wmf";

namespace lyx {

namespace frontend {


GuiClipboard::GuiClipboard()
{
	connect(qApp->clipboard(), SIGNAL(dataChanged()),
		this, SLOT(on_dataChanged()));
	// initialize clipboard status.
	on_dataChanged();
}


string const GuiClipboard::getAsLyX() const
{
	LYXERR(Debug::ACTION, "GuiClipboard::getAsLyX(): `");
	// We don't convert encodings here since the encoding of the
	// clipboard contents is specified in the data itself
	QMimeData const * source =
		qApp->clipboard()->mimeData(QClipboard::Clipboard);
	if (!source) {
		LYXERR(Debug::ACTION, "' (no QMimeData)");
		return string();
	}

	if (source->hasFormat(lyx_mime_type)) {
		// data from ourself or some other LyX instance
		QByteArray const ar = source->data(lyx_mime_type);
		string const s(ar.data(), ar.count());
		LYXERR(Debug::ACTION, s << "'");
		return s;
	}
	LYXERR(Debug::ACTION, "'");
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
	
	LASSERT(!types.empty(), /**/);
	
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
	string document_path = cur.buffer().fileName().onlyPath().absFilename();
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
		dlg.save(toqstr(filename.onlyPath().absFilename()), filter,
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
		if (!suffixIs(ascii_lowercase(filename.absFilename()),
			      "." + extensions[type])) {
			// the user changed the extension. Check if the type is available
			size_t i;
			for (i = 1; i != types.size(); ++i) {
				if (suffixIs(ascii_lowercase(filename.absFilename()),
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
			from_utf8(filename.absFilename())), 1, 1, _("&Overwrite"), _("&Cancel"));
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
			LYXERR(Debug::ACTION, "No image in clipboard");
			return FileName();
		}

		// convert into graphics format
		QByteArray ar;
		QBuffer buffer(&ar);
		buffer.open(QIODevice::WriteOnly);
		if (type == PngGraphicsType)
			image.save(toqstr(filename.absFilename()), "PNG");
		else if (type == JpegGraphicsType)
			image.save(toqstr(filename.absFilename()), "JPEG");
		else
			LASSERT(false, /**/);
		
		return filename;
	}
	
	// get mime data
	QMimeData const * source =
	qApp->clipboard()->mimeData(QClipboard::Clipboard);
	if (!source) {
		LYXERR(Debug::ACTION, "0 bytes (no QMimeData)");
		return FileName();
	}
	
	// get mime for type
	QString mime;
	switch (type) {
	case PdfGraphicsType: mime = pdf_mime_type; break;
	case LinkBackGraphicsType: mime = pdf_mime_type; break;
	case EmfGraphicsType: mime = emf_mime_type; break;
	case WmfGraphicsType: mime = wmf_mime_type; break;
	default: LASSERT(false, /**/);
	}
	
	// get data
	if (!source->hasFormat(mime))
		return FileName();
	// data from ourself or some other LyX instance
	QByteArray const ar = source->data(mime);
	LYXERR(Debug::ACTION, "Getting from clipboard: mime = " << mime.data()
	       << "length = " << ar.count());
	
	QFile f(toqstr(filename.absFilename()));
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		LYXERR(Debug::ACTION, "Error opening file "
		       << filename.absFilename() << " for writing");
		return FileName();
	}
	
	// write the (LinkBack) PDF data
	f.write(ar);
	if (type == LinkBackGraphicsType) {
#ifdef Q_WS_MACX
		void const * linkBackData;
		unsigned linkBackLen;
		getLinkBackData(&linkBackData, &linkBackLen);
		f.write((char *)linkBackData, linkBackLen);
		quint32 pdfLen = ar.size();
		QDataStream ds(&f);
		ds << pdfLen; // big endian by default
#else
		// only non-Mac this should never happen
		LASSERT(false, /**/);
#endif // Q_WS_MACX
	}

	f.close();
	return filename;
}


docstring const GuiClipboard::getAsText() const
{
	// text data from other applications
	QString const str = qApp->clipboard()->text(QClipboard::Clipboard)
				.normalized(QString::NormalizationForm_C);
	LYXERR(Debug::ACTION, "GuiClipboard::getAsText(): `" << str << "'");
	if (str.isNull())
		return docstring();

	return internalLineEnding(qstring_to_ucs4(str));
}


void GuiClipboard::put(string const & lyx, docstring const & text)
{
	LYXERR(Debug::ACTION, "GuiClipboard::put(`" << lyx << "' `"
			      << to_utf8(text) << "')");
	// We don't convert the encoding of lyx since the encoding of the
	// clipboard contents is specified in the data itself
	QMimeData * data = new QMimeData;
	if (!lyx.empty()) {
		QByteArray const qlyx(lyx.c_str(), lyx.size());
		data->setData(lyx_mime_type, qlyx);
	}
	// Don't test for text.empty() since we want to be able to clear the
	// clipboard.
	QString const qtext = toqstr(text);
	data->setText(qtext);
	qApp->clipboard()->setMimeData(data, QClipboard::Clipboard);
}


bool GuiClipboard::hasLyXContents() const
{
	QMimeData const * const source =
		qApp->clipboard()->mimeData(QClipboard::Clipboard);
	return source && source->hasFormat(lyx_mime_type);
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

	QMimeData const * const source =
	qApp->clipboard()->mimeData(QClipboard::Clipboard);

	// handle image cases first
	if (type == PngGraphicsType || type == JpegGraphicsType)
		return source->hasImage();

	// handle LinkBack for Mac
	if (type == LinkBackGraphicsType)
#ifdef Q_WS_MACX
		return isLinkBackDataInPasteboard();
#else
		return false;
#endif // Q_WS_MACX
	
	// get mime data
	QStringList const & formats = source->formats();
	LYXERR(Debug::ACTION, "We found " << formats.size() << " formats");
	for (int i = 0; i < formats.size(); ++i)
		LYXERR(Debug::ACTION, "Found format " << formats[i]);

	// compute mime for type
	QString mime;
	switch (type) {
	case EmfGraphicsType: mime = emf_mime_type; break;
	case WmfGraphicsType: mime = wmf_mime_type; break;
	case PdfGraphicsType: mime = pdf_mime_type; break;
	default: LASSERT(false, /**/);
	}
	
	return source && source->hasFormat(mime);
}


bool GuiClipboard::isInternal() const
{
	// ownsClipboard() is also true for stuff coming from dialogs, e.g.
	// the preamble dialog
	// FIXME: This does only work on X11, since ownsClipboard() is
	// hardwired to return false on Windows and OS X.
	return qApp->clipboard()->ownsClipboard() && hasLyXContents();
}


bool GuiClipboard::hasInternal() const
{
	// Windows and Mac OS X does not have the concept of ownership;
	// the clipboard is a fully global resource so all applications 
	// are notified of changes.
#if (defined(Q_WS_X11))
	return true;
#else
	return false;
#endif
}


void GuiClipboard::on_dataChanged()
{
	QMimeData const * const source =
	qApp->clipboard()->mimeData(QClipboard::Clipboard);
	QStringList l = source->formats();
	LYXERR(Debug::ACTION, "Qt Clipboard changed. We found the following mime types:");
	for (int i = 0; i < l.count(); i++)
		LYXERR(Debug::ACTION, l.value(i));
	
	text_clipboard_empty_ = qApp->clipboard()->
		text(QClipboard::Clipboard).isEmpty();

	has_lyx_contents_ = hasLyXContents();
	has_graphics_contents_ = hasGraphicsContents();
}


bool GuiClipboard::empty() const
{
	// We need to check both the plaintext and the LyX version of the
	// clipboard. The plaintext version is empty if the LyX version
	// contains only one inset, and the LyX version is empty if the
	// clipboard does not come from LyX.
	if (!text_clipboard_empty_)
		return false;
	return !has_lyx_contents_ && !has_graphics_contents_;
}

} // namespace frontend
} // namespace lyx

#include "GuiClipboard_moc.cpp"
