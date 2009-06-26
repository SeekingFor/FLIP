#ifndef _sqlite3dbstatement_
#define _sqlite3dbstatement_

#include "sqlite3db.h"

#include <vector>
#include <map>

namespace SQLite3DB
{

class Statement
{
public:
	Statement();
	Statement(sqlite3_stmt *statement);
	Statement(const Statement &rhs);
	virtual ~Statement();

	virtual const int ParameterCount() { return m_parametercount; }
	virtual const int ResultColumnCount() { return m_resultcolumncount; }

	virtual const bool Valid();

	virtual void Finalize();

	virtual const bool Reset();
	virtual const bool Step(const bool saveinsertrowid=false);

	virtual const bool RowReturned() { return m_rowreturned; }

	virtual const long GetLastInsertRowID()	{ return m_lastinsertrowid; }

	// both Bind and Result have column index starting at 0
	// Blob results are not copied, user must make a copy in memory if it needs to be used in the future

	virtual const bool Bind(const int column);
	virtual const bool Bind(const int column, const int value);
	virtual const bool Bind(const int column, const long value)			{ return Bind(column,static_cast<int>(value)); }
	virtual const bool Bind(const int column, const double value);
	virtual const bool Bind(const int column, const std::string &value);
	virtual const bool Bind(const int column, const void *data, const int length);

	virtual const bool ResultNull(const int column);
	virtual const bool ResultInt(const int column, int &result);
	virtual const bool ResultDouble(const int column, double &result);
	virtual const bool ResultText(const int column, std::string &result);
	virtual const bool ResultBlob(const int column, void *data, int &length);
	
	Statement &operator=(const Statement &rhs);

private:
	sqlite3_stmt *m_statement;
	int m_parametercount;
	int m_resultcolumncount;
	bool m_rowreturned;
	long m_lastinsertrowid;

	static std::map<sqlite3_stmt *, long> m_statementcount;

};	//class

}	// namespace

#endif	// _sqlite3dbstatement_
