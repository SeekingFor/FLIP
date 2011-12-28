#include "option.h"
#include "db/sqlite3db.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const bool Option::Get(const std::string &option, std::string &value)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT OptionValue FROM tblOption WHERE Option=?;");
	st.Bind(0,option);
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,value);
		return true;
	}
	else
	{
		return false;
	}
}

const bool Option::GetBool(const std::string &option, bool &value)
{
	std::string valstr="";
	if(Get(option,valstr))
	{
		if(valstr=="TRUE" || valstr=="true" || valstr=="1")
		{
			value=true;
			return true;
		}
		else if(valstr=="FALSE" || valstr=="false" || valstr=="0")
		{
			value=false;
			return true;
		}
	}

	return false;

}

const bool Option::GetInt(const std::string &option, int &value)
{
	std::string valstr="";
	if(Get(option,valstr))
	{
		std::istringstream istr(valstr);
		if(istr >> value)
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

const bool Option::GetUnsignedShort(const std::string &option, unsigned short &value)
{
	std::string valstr="";
	if(Get(option,valstr))
	{
		std::istringstream istr(valstr);
		if(istr >> value)
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
