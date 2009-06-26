#ifndef _logfile_
#define _logfile_

#include <string>

class LogFile
{
public:
	LogFile();
	LogFile(const std::string &filename);
	~LogFile();

	enum LogLevel
	{
		LOGLEVEL_FATAL=0,
		LOGLEVEL_ERROR=1,
		LOGLEVEL_WARNING=2,
		LOGLEVEL_INFO=3,
		LOGLEVEL_DEBUG=4,
		LOGLEVEL_TRACE=5
	};
	
	bool OpenFile();
	bool CloseFile();
	
	std::string GetFileName() { return m_filename; }
	void SetFileName(std::string filename) { m_filename=filename; }
	
	const bool GetWriteDate() { return m_writedate; }
	void SetWriteDate(const bool writedate) { m_writedate=writedate; }
	
	const bool GetWriteLogLevel() { return m_writeloglevel; }
	void SetWriteLogLevel(const bool writeloglevel) { m_writeloglevel=writeloglevel; }
	
	const LogLevel GetLogLevel() { return m_loglevel; }
	void SetLogLevel(const LogLevel loglevel) { m_loglevel=loglevel; }

	const bool GetWriteNewLine() { return m_writenewline; }
	void SetWriteNewLine(const bool writenewline) { m_writenewline=writenewline; }
	
	//void WriteLog(const char *format, ...);
	void WriteLog(const std::string &text);
	//void WriteLog(const LogLevel level, const char *format, ...);
	void WriteLog(const LogLevel level, const std::string &text);

	void Fatal(const std::string &text)		{ WriteLog(LOGLEVEL_FATAL,text); }
	void Error(const std::string &text)		{ WriteLog(LOGLEVEL_ERROR,text); }
	void Warn(const std::string &text)		{ WriteLog(LOGLEVEL_WARNING,text); }
	void Info(const std::string &text)		{ WriteLog(LOGLEVEL_INFO,text); }
	void Debug(const std::string &text)		{ WriteLog(LOGLEVEL_DEBUG,text); }
	void Trace(const std::string &text)		{ WriteLog(LOGLEVEL_TRACE,text); }

	void WriteNewLine();
	
private:
	void WriteDate();
	void WriteLogLevel(LogLevel level);

	FILE *m_fileptr;
	std::string m_filename;
	LogLevel m_loglevel;
	bool m_writedate;
	bool m_writeloglevel;
	bool m_writenewline;
	char *m_datebuffer;
	
};

#endif	// _logfile_
