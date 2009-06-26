#include "sqlite3recordset.h"

#include <cstdlib>

#ifdef XMEM
	#include <xmem.h>
#endif

namespace SQLite3DB
{

Recordset::Recordset()
{
	m_rs=NULL;
	m_rows=0;
	m_cols=0;
	m_currentrow=0;
}

Recordset::Recordset(char **rs, int rows, int cols)
{
	m_rs=rs;
	m_rows=rows;
	m_cols=cols;
	m_currentrow=0;
}

Recordset::~Recordset()
{
	Free();
}

const char *Recordset::Get(const int row, const int field)
{
	if(row>=0 && row<m_rows && field>=0 && field<m_cols)
	{
		return m_rs[m_cols+(m_cols*row)+field];
	}
	else
	{
		return NULL;
	}
}

const char *Recordset::GetColumnName(const int column)
{
	if(column>=0 && column<m_cols)
	{
		return m_rs[column];
	}
	else
	{
		return NULL;
	}
}

const double Recordset::GetDouble(const int field)
{
	const char *result=GetField(field);
	if(result)
	{
		return atof(result);
	}
	else
	{
		return 0;
	}
}

const char *Recordset::GetField(const int field)
{
	return Get(m_currentrow,field);
}

const int Recordset::GetInt(const int field)
{
	const char *result=GetField(field);
	if(result)
	{
		return atoi(result);
	}
	else
	{
		return 0;
	}
}

void Recordset::Open(const std::string &sql, DB *db)
{
	Free();
	m_currentrow=0;

	if(sqlite3_get_table(db->GetDB(),sql.c_str(),&m_rs,&m_rows,&m_cols,NULL)==SQLITE_OK)
	{
	}
}

}	// namespace
