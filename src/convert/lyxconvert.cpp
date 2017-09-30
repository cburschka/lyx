/*
set cflags=`env PKG_CONFIG_PATH=/usr/local/qt5/lib/pkgconfig pkg-config --cflags Qt5Widgets`
set libs=`env PKG_CONFIG_PATH=/usr/local/qt5/lib/pkgconfig pkg-config --libs --static Qt5Widgets`
g++ -std=gnu++11 $cflags lyxconvert.cpp -o lyxconvert $libs
*/
#include <iostream>
#include <QApplication>
#include <QImage>
#include <QFile>
#include <QPainter>
#include <QPdfWriter>


const char * basename(const char * name)
{
#ifdef Q_OS_WIN
	const char * slashpos = strrchr(name, '\\');
#else
	const char * slashpos = strrchr(name, '/');
#endif

	if (NULL != slashpos) name = ++slashpos ;
	return name;
}


void usage(const char * name)
{
	std::cerr << "Usage: " << name
		<< " [-f infmt] [-t outfmt] input output" << std::endl;
	exit(1);
}


void version(const char * name)
{
	std::cerr << name << ": version 1.0" << std::endl;
	exit(0);
}


bool isFileExt(const char * name, const char * ext)
{
	const char * dotpos = strrchr(name, '.');
	return NULL != dotpos && !strcmp(++dotpos, ext);
}


int main(int argc, char **argv)
{
	int arg = 1;
	const char * iformat  = NULL;
	const char * oformat  = NULL;
	const char * infile   = NULL;
	const char * outfile  = NULL;
	const char * myname   = basename(argv[0]);
	char * qtargs[] = {
		argv[0],
		(char*)"-platform", (char*)"minimal",
		NULL };
	int  qtargsc = sizeof(qtargs) / sizeof(qtargs[0]) - 1;
	bool debug = (1 == 0);

	while (arg < argc) {
		if ('-' == argv[arg][0] && !strcmp(argv[arg], "-platform")) {
			qtargs[2] = argv[++arg]; arg++ ;
		} else if ('-' == argv[arg][0] && 'f' == argv[arg][1]) {
			iformat = argv[++arg]; arg++ ;
		} else if ('-' == argv[arg][0] && 't' == argv[arg][1]) {
			oformat = argv[++arg]; arg++ ;
		} else if ('-' == argv[arg][0] && 'd' == argv[arg][1]) {
			debug = (1 == 1); arg++;
		} else if ('-' == argv[arg][0] && 'V' == argv[arg][1]) {
			version(myname);
		} else if ('-' == argv[arg][0]) {
			usage(myname);
		} else if (NULL == infile) {
			infile = argv[arg++];
		} else if (NULL == outfile) {
			outfile = argv[arg++];
			if (NULL == oformat) {
				if (isFileExt(outfile, "pdf")) {
					oformat = "pdf";
				} else if (isFileExt(outfile, "eps")) {
					oformat = "eps";
				}
			}
		} else {
			usage(myname);
		}
	}
	if (NULL == infile || NULL == outfile) {
		usage(myname);
	}

	QApplication app(qtargsc, &qtargs[0]);
	QFile ifile(QString::fromLocal8Bit(infile));
	QImage img;

	if (debug) {
		std::cerr << myname << ": platform is " << (NULL == qtargs[2] ? "default" : qtargs[2]) << std::endl;
	}

	if (debug) {
		std::cerr << myname << ": Load file '" << infile <<
			"', infmt is '" << (NULL == iformat ? "auto" : iformat) << "'" << std::endl;
	}
	if (!ifile.exists()) {
		std::cerr << myname << ": Image file '" << infile << "' doesn't exist" << std::endl;
		return 2;
	} else if (!img.load(ifile.fileName(), iformat)) {
		std::cerr << myname << ": Cannot load image '" << infile << "'" << std::endl;
		return 3;
	}

	if (debug) {
		std::cerr << myname << ": Save converted image to file '" << outfile <<
			"', outfmt is '" << (NULL == oformat ? "auto" : oformat) << "'" << std::endl;
	}
	if (NULL != oformat && !strcmp(oformat, "eps")) {
		std::cerr << myname << ": Conversion of images to format '" << oformat << "' is not supported" << std::endl;
		return 4;
	} else if (NULL != oformat && !strcmp(oformat, "pdf")) {
#if (QT_VERSION >= 0x050300)
		QSize size = img.size();
		QPdfWriter pdfwriter(QString::fromLocal8Bit(outfile));
		int dpi = pdfwriter.logicalDpiX();
		QPageSize pagesize(size * qreal(72.0 / dpi));
		QMarginsF margins(0, 0, 0, 0);
		QPageLayout pagelayout(pagesize, QPageLayout::Portrait, margins);
		pdfwriter.setPageLayout(pagelayout);
		QPainter painter(&pdfwriter);
		painter.drawImage(0, 0, img);
		painter.end();
#else
		std::cerr << myname << ": Conversion of images to format '" << oformat << "' is not supported" << std::endl;
		return 4;
#endif
	} else if (!img.save(QString::fromLocal8Bit(outfile), oformat)) {
		std::cerr << myname << ": Cannot save converted image to '" << outfile << "'" << std::endl;
		return 5;
	}
	return 0;
}

