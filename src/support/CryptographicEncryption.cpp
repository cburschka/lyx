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

#include <config.h>

#include "CryptographicEncryption.h"

#include <QDataStream>


#ifdef LYX_ENCRYPTION
#include <openssl/aes.h>
#include <openssl/evp.h>
#endif


namespace lyx {
namespace support {



CryptographicEncryption::CryptographicEncryption()
{
}


int CryptographicEncryption::blockAlign(int blockSize, QByteArray& bytes)
{
	int pad = 2 * blockSize - (bytes.size() % blockSize); // pad at least one block
	bytes.append(QByteArray(pad, (char)pad));
	return pad;
}


int CryptographicEncryption::blockDealign(QByteArray& bytes)
{
	int size = bytes.size();
	if (size == 0)
		return 0;
	char padded = bytes.at(size - 1);
	bytes.resize(size - padded);
	return padded;
}


bool CryptographicEncryption::aesEnryption(QByteArray const & in, QByteArray* out, QByteArray const & key, bool encrypt)
{
#ifndef LYX_ENCRYPTION
	(void) in;
	(void) out;
	(void) key;
	(void) encrypt;
	return false;
#else
	if (!out)
		return false;

	int keySize = key.size();
	if (keySize != 16 && keySize != 24 && keySize != 32) {
		return false;
	}

	// AES needs aligned data, but we must not touch already encrypted data
	QByteArray aligned = in;
	if (encrypt) {
		blockAlign(AES_BLOCK_SIZE, aligned);
	}
	if ((aligned.size() % AES_BLOCK_SIZE) != 0) {
		return false;
	}

	*out = QByteArray(aligned.size(), 0);
	AES_KEY aeskey;
	if (encrypt)
		AES_set_encrypt_key((unsigned char*)key.constData(), keySize * 8, &aeskey);
	else
		AES_set_decrypt_key((unsigned char*)key.constData(), keySize * 8, &aeskey);

	// use some arbitrary start values
	unsigned char iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	AES_cbc_encrypt((unsigned char*)aligned.constData(),
					(unsigned char*)out->data(),
					aligned.size(), &aeskey, iv,
					(encrypt ? AES_ENCRYPT : AES_DECRYPT));

	if (!encrypt)
		blockDealign(*out);

	return true;
#endif
}


QByteArray CryptographicEncryption::hash(QByteArray const & bytes, QCryptographicHash::Algorithm algorithm)
{
	QCryptographicHash hashAlgo(algorithm);
	hashAlgo.addData(bytes);
	return hashAlgo.result();
}


bool CryptographicEncryption::encyrpt(QByteArray const & plain, QByteArray* encrypted, QByteArray const & key)
{
	if (!encrypted)
		return false;

	QByteArray bytes;
	QDataStream stream(&bytes, QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_4_6);
	stream << plain;
	stream << hash(plain, QCryptographicHash::Md5);

	if (!aesEnryption(bytes, encrypted, key, true)) {
		encrypted->clear();
		return false;
	}

	return true;
}


bool CryptographicEncryption::decyrpt(QByteArray const & encrypted, QByteArray* plain, QByteArray const & key)
{
	if (!plain)
		return false;

	QByteArray bytes;
	if (!aesEnryption(encrypted, &bytes, key, false))
		return false;

	QByteArray decryptedHash;
	QDataStream stream(bytes);
	stream.setVersion(QDataStream::Qt_4_6);
	stream >> *plain;
	stream >> decryptedHash;

	if (decryptedHash != hash(*plain, QCryptographicHash::Md5)) {
		plain->clear();
		return false;
	}

	return true;
}


QByteArray CryptographicEncryption::bytesToKey(QByteArray const & bytes)
{
#ifndef LYX_ENCRYPTION
	(void) bytes;
	return QByteArray();
#else
	const char* in = bytes.constData();
	int iterations = 10000; // here we could adjust our paranoija
	unsigned char out[64];
	PKCS5_PBKDF2_HMAC_SHA1(in, bytes.size(), 0, 0, iterations, 32, out);
	
	return QByteArray((const char*) out, 32);
#endif
}


QByteArray CryptographicEncryption::stringToKey(QString const & str)
{
	QByteArray utf8 = str.toUtf8();
	return bytesToKey(utf8);
}





}
}
