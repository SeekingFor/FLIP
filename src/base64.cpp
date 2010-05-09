#include "base64.h"

#include <polarssl/base64.h>

const bool Base64::Decode(const std::string &encoded, std::vector<unsigned char> &data)
{
	if(encoded.size()>0)
	{
		int rval=0;
		int datalength=0;

		rval=base64_decode(0,&datalength,(unsigned char *)encoded.c_str(),encoded.size());

		data.resize(datalength,0);

		rval=base64_decode(&data[0],&datalength,(unsigned char *)encoded.c_str(),encoded.size());
		if(datalength<data.size())
		{
			data.erase(data.begin()+datalength,data.end());
		}

		if(rval==0)
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
		data.clear();
		return true;
	}
}

const bool Base64::Encode(std::vector<unsigned char> &data, std::string &encoded)
{
	if(data.size()>0)
	{
		int rval=0;
		int encodedlength=0;
		std::vector<unsigned char> encodeddata;

		rval=base64_encode(0,&encodedlength,&data[0],data.size());
		encodeddata.resize(encodedlength,0);

		rval=base64_encode(&encodeddata[0],&encodedlength,&data[0],data.size());

		if(rval==0)
		{
			if(encodeddata.size()>0)
			{
				encoded=std::string(encodeddata.begin(),encodeddata.end()-1);
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		encoded="";
		return true;
	}
}
