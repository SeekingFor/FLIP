#include "rsakeypair.h"
#include "base64.h"
#include "stringfunctions.h"

#include <polarssl/bignum.h>

#include <cstring>
#include <vector>
#include <cmath>

RSAKeyPair::RSAKeyPair():m_initialized(false)
{
	havege_init(&m_hs);
}

RSAKeyPair::~RSAKeyPair()
{
	FreeContext();
}

const bool RSAKeyPair::Decrypt(const std::string &encrypted, std::string &message)
{
	if(m_initialized)
	{
		std::vector<std::string> encryptedparts;

		StringFunctions::Split(encrypted,"|",encryptedparts);
		message="";

		for(std::vector<std::string>::iterator i=encryptedparts.begin(); i!=encryptedparts.end(); i++)
		{
			std::vector<unsigned char> input;
			std::vector<unsigned char> output(mpi_size(&m_rsa.N),0);
			int outputlength=output.size();
			Base64::Decode((*i),input);

			if(input.size()==mpi_size(&m_rsa.N))
			{
				if(rsa_pkcs1_decrypt(&m_rsa,RSA_PRIVATE,&outputlength,&input[0],&output[0],output.size())==0)
				{
					if(outputlength<output.size())
					{
						output.erase(output.begin()+outputlength,output.end());
					}
					message+=std::string(output.begin(),output.end());
				}
			}
			else
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

const bool RSAKeyPair::Encrypt(const std::string &message, std::string &encrypted)
{
	if(m_initialized)
	{
		std::vector<unsigned char> output(mpi_size(&m_rsa.N),0);
		long blocksize=output.size()-11;
		std::string encryptedoutput("");
		std::string mess=message;
		std::vector<std::string> messageblocks;

		while(mess.size()>0)
		{
			if(mess.size()>blocksize)
			{
				messageblocks.push_back(mess.substr(0,blocksize));
				mess.erase(0,blocksize);
			}
			else
			{
				messageblocks.push_back(mess);
				mess.erase(0);
			}
		}

		for(std::vector<std::string>::iterator i=messageblocks.begin(); i!=messageblocks.end(); i++)
		{
			std::vector<unsigned char> input((*i).begin(),(*i).end());
			if(rsa_pkcs1_encrypt(&m_rsa,RSA_PUBLIC,input.size(),&input[0],&output[0])==0)
			{
				std::string temp("");
				Base64::Encode(output,temp);
				if(i!=messageblocks.begin())
				{
					encryptedoutput+="|";
				}
				encryptedoutput+=temp;
			}
			else
			{
				return false;
			}
		}

		encrypted=encryptedoutput;

		return true;

	}
	else
	{
		return false;
	}
}

void RSAKeyPair::FreeContext()
{
	if(m_initialized)
	{
		rsa_free(&m_rsa);
		m_initialized=false;
	}
}

const bool RSAKeyPair::Generate()
{
	InitializeContext();

	int rval=rsa_gen_key(&m_rsa,1024,65537);

	if(rval!=0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

const std::string RSAKeyPair::GetEncodedPublicKey()
{
	if(m_initialized)
	{
		int rval=0;
		std::string lenstr("");
		std::string nencoded("");
		std::string eencoded("");
		std::vector<unsigned char> n(mpi_size(&m_rsa.N),0);
		std::vector<unsigned char> e(mpi_size(&m_rsa.E),0);

		rval=mpi_write_binary(&m_rsa.N,&n[0],n.size());
		rval|=mpi_write_binary(&m_rsa.E,&e[0],e.size());

		Base64::Encode(n,nencoded);
		Base64::Encode(e,eencoded);

		StringFunctions::Convert(m_rsa.len,lenstr);

		return std::string(lenstr+"|"+nencoded+"|"+eencoded);
	}
	else
	{
		return std::string("");
	}
}

const std::string RSAKeyPair::GetEncodedPrivateKey()
{
	if(m_initialized)
	{
		std::string dencoded("");
		std::string pencoded("");
		std::string qencoded("");
		std::string dpencoded("");
		std::string dqencoded("");
		std::string qpencoded("");
		std::vector<unsigned char> d(mpi_size(&m_rsa.D),0);
		std::vector<unsigned char> p(mpi_size(&m_rsa.P),0);
		std::vector<unsigned char> q(mpi_size(&m_rsa.Q),0);
		std::vector<unsigned char> dp(mpi_size(&m_rsa.DP),0);
		std::vector<unsigned char> dq(mpi_size(&m_rsa.DQ),0);
		std::vector<unsigned char> qp(mpi_size(&m_rsa.QP),0);

		mpi_write_binary(&m_rsa.D,&d[0],d.size());
		mpi_write_binary(&m_rsa.P,&p[0],p.size());
		mpi_write_binary(&m_rsa.Q,&q[0],q.size());
		mpi_write_binary(&m_rsa.DP,&dp[0],dp.size());
		mpi_write_binary(&m_rsa.DQ,&dq[0],dq.size());
		mpi_write_binary(&m_rsa.QP,&qp[0],qp.size());

		Base64::Encode(d,dencoded);
		Base64::Encode(p,pencoded);
		Base64::Encode(q,qencoded);
		Base64::Encode(dp,dpencoded);
		Base64::Encode(dq,dqencoded);
		Base64::Encode(qp,qpencoded);

		return std::string(GetEncodedPublicKey()+"|"+dencoded+"|"+pencoded+"|"+qencoded+"|"+dpencoded+"|"+dqencoded+"|"+qpencoded);
	}
	else
	{
		return std::string("");
	}
}


void RSAKeyPair::InitializeContext()
{
	FreeContext();

	rsa_init(&m_rsa,RSA_PKCS_V15,0,havege_rand,&m_hs);
	m_initialized=true;
}

const bool RSAKeyPair::SetFromEncodedPublicKey(const std::string &publickey)
{
	InitializeContext();
	std::vector<std::string> keyparts;

	StringFunctions::Split(publickey,"|",keyparts);

	if(keyparts.size()==3)
	{
		std::vector<unsigned char> n;
		std::vector<unsigned char> e;

		StringFunctions::Convert(keyparts[0],m_rsa.len);
		Base64::Decode(keyparts[1],n);
		Base64::Decode(keyparts[2],e);

		mpi_read_binary(&m_rsa.N,&n[0],n.size());
		mpi_read_binary(&m_rsa.E,&e[0],e.size());

		if(rsa_check_pubkey(&m_rsa)==0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	
}

const bool RSAKeyPair::SetFromEncodedPrivateKey(const std::string &privatekey)
{
	InitializeContext();
	std::vector<std::string> keyparts;

	StringFunctions::Split(privatekey,"|",keyparts);

	if(keyparts.size()==9)
	{	
		if(SetFromEncodedPublicKey(keyparts[0]+"|"+keyparts[1]+"|"+keyparts[2])==true)
		{
			std::vector<unsigned char> d;
			std::vector<unsigned char> p;
			std::vector<unsigned char> q;
			std::vector<unsigned char> dp;
			std::vector<unsigned char> dq;
			std::vector<unsigned char> qp;

			Base64::Decode(keyparts[3],d);
			Base64::Decode(keyparts[4],p);
			Base64::Decode(keyparts[5],q);
			Base64::Decode(keyparts[6],dp);
			Base64::Decode(keyparts[7],dq);
			Base64::Decode(keyparts[8],qp);

			mpi_read_binary(&m_rsa.D,&d[0],d.size());
			mpi_read_binary(&m_rsa.P,&p[0],p.size());
			mpi_read_binary(&m_rsa.Q,&q[0],q.size());
			mpi_read_binary(&m_rsa.DP,&dp[0],dp.size());
			mpi_read_binary(&m_rsa.DQ,&dq[0],dq.size());
			mpi_read_binary(&m_rsa.QP,&qp[0],qp.size());

			if(rsa_check_privkey(&m_rsa)==0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}
