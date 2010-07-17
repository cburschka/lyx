// -*- C++ -*-
/**
 * \file CryptographicEncryption.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CRYPTOCRAPHIC_ENCYPTION_H
#define CRYPTOCRAPHIC_ENCYPTION_H

#include <QCryptographicHash>

namespace lyx {
namespace support {


class CryptographicEncryption
{
public:
	CryptographicEncryption();

	enum Keytype {
		Password = 0,
		KeytypeCount
	};

	bool encyrpt(QByteArray const & plain, QByteArray * encrypted, QByteArray const & key);
	bool decyrpt(QByteArray const & encrypted, QByteArray * plain, QByteArray const & key);

	QByteArray bytesToKey(QByteArray const &);
	QByteArray stringToKey(QString const &);

private:
	int blockAlign(int blockSize, QByteArray& bytes);
	int blockDealign(QByteArray& bytes);
	QByteArray hash(QByteArray const & bytes, QCryptographicHash::Algorithm);
	bool aesEnryption(QByteArray const & in, QByteArray* out, QByteArray const & key, bool encrypt);
};


}
}

#endif
