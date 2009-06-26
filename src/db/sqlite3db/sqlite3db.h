#ifndef _sqlite3db_
#define _sqlite3db_

#include <sqlite3.h>
#include <string>
#include "../sqlite3db.h"

#if SQLITE_VERSION_NUMBER<3006006
#error "Your version of SQLite is too old!  3.6.6.2 or later is required."
#endif

#if SQLITE_VERSION_NUMBER==3006014
#error "SQLite 3.6.14 is not supported!"
#endif

namespace SQLite3DB
{

class DB
{
public:
	DB();
	DB(const std::string &filename);
	~DB();
	
	const bool Open(const std::string &filename);
	const bool Close();
	
	const int GetLastResult() { return m_lastresult; }	// gets result of last action taken - standard sqlite3 return codes
	const int GetLastError(std::string &errormessage);	// gets last error of this database
	
	const bool IsOpen();
	
	const bool Execute(const std::string &sql);	// executes a statement returing true if successful
	const bool ExecuteInsert(const std::string &sql, long &insertid);	// call when inserting data and the insertid of the row inserted is needed, otherwise Execute can be called if the row id is not needed
	Recordset Query(const std::string &sql);		// executes a statement returning a recordset
	Statement Prepare(const std::string &sql);	// prepares a statement returning the statement object

	const int SetBusyTimeout(const int ms);		// sets busy timeout in ms.  SQLite will wait for a lock up to this many ms before returning SQLITE_BUSY

	sqlite3 *GetDB() { return m_db; }

private:
	void Initialize();
	
	sqlite3 *m_db;
	int m_lastresult;
};

}	// namespace

#endif	// _sqlite3db_
