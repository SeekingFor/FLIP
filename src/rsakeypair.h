#ifndef _rsakeypair_
#define _rsakeypair_

#include <polarssl/rsa.h>
#include <polarssl/havege.h>

#include <string>

class RSAKeyPair
{
public:
	RSAKeyPair();
	~RSAKeyPair();

	const bool Generate();

	const bool SetFromEncodedPublicKey(const std::string &publickey);
	const bool SetFromEncodedPrivateKey(const std::string &privatekey);

	const std::string GetEncodedPublicKey();
	const std::string GetEncodedPrivateKey();

	const bool Encrypt(const std::string &message, std::string &encrypted);
	const bool Decrypt(const std::string &encrypted, std::string &message);

private:
	void InitializeContext();
	void FreeContext();

	havege_state m_hs;
	rsa_context m_rsa;
	bool m_initialized;

};

#endif	// _rsakeypair_
