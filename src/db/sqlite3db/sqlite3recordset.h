#ifndef _sqlite3recordset_
#define _sqlite3recordset_

#include "sqlite3db.h"

namespace SQLite3DB
{

class Recordset
{
public:
	Recordset();
	Recordset(char **rs, int rows, int cols);
	virtual ~Recordset();

	virtual void Free() { if(m_rs) { sqlite3_free_table(m_rs); m_rs=NULL; } }
	virtual const bool Empty() const 		{ return (m_rs==NULL || m_rows==0) ? true : false ; }

	virtual const int Count() const			{ return m_rows; }
	virtual const bool AtBeginning() const	{ return m_currentrow==0; }
	virtual const bool AtEnd() const		{ return m_currentrow>=m_rows; }
	virtual const int Cols() const			{ return m_cols; }

	virtual const bool Next() { if(m_currentrow<m_rows) { m_currentrow++; return true; } else { return false; } }
	virtual const bool Previous() { if(m_currentrow-1>=0) { m_currentrow--; return true; } else { return false; } }
	virtual void Beginning() { m_currentrow=0; }

	virtual const char *Get(const int row, const int field);
	virtual const char *GetField(const int field);
	virtual const int GetInt(const int field);
	virtual const double GetDouble(const int field);
	virtual const char *GetColumnName(const int column);

	virtual void Open(const std::string &sql, DB *db);

private:
	char **m_rs;
	int m_rows;
	int m_cols;
	int m_currentrow;

};	// class Recordset

}	// namespace

#endif	// _sqlite3recordset_
