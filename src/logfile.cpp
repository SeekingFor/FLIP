#include "logfile.h"

#include <ctime>
#include <cstdarg>

#ifdef XMEM
	#include <xmem.h>
#endif

LogFile::LogFile()
{
	m_fileptr=NULL;
	m_loglevel=LOGLEVEL_DEBUG;
	m_writedate=false;
	m_writeloglevel=false;
	m_writenewline=true;
	m_datebuffer=new char[100];
}

LogFile::LogFile(const std::string &filename)
{
	m_fileptr=NULL;	
	m_loglevel=LOGLEVEL_DEBUG;
	m_writedate=false;
	m_writeloglevel=false;
	m_filename=filename;
	m_writenewline=true;
	m_datebuffer=new char[100];
}

LogFile::~LogFile()
{
	CloseFile();
	delete [] m_datebuffer;
}

bool LogFile::CloseFile()
{
	if(m_fileptr)
	{
		fclose(m_fileptr);
		m_fileptr=NULL;
	}

	return true;
}

bool LogFile::OpenFile()
{
	CloseFile();

	m_fileptr=fopen(m_filename.c_str(),"a+b");

	if(m_fileptr)
	{
		return true;
	}
	else
	{
		return false;	
	}
}

void LogFile::WriteDate()
{
	if(m_fileptr)
	{
		time_t rawtime=time(NULL);
		struct tm *timeinfo=gmtime(&rawtime);
		
		strftime(m_datebuffer,99,"%Y-%m-%d %H:%M:%S : ",timeinfo);
		m_datebuffer[99]='\0';
		
		fputs(m_datebuffer,m_fileptr);
	}
}
/*
void LogFile::WriteLog(const char *format, ...)
{
	va_list va;
	va_start(va,format);
	
	if(!m_fileptr)
	{
		OpenFile();
	}
	
	if(m_fileptr)
	{
		if(m_writedate)
		{
			WriteDate();
		}
		vfprintf(m_fileptr,format,va);
		if(m_writenewline==true)
		{
			fputs("\r\n",m_fileptr);
		}
		fflush(m_fileptr);
	}
	
	va_end(va);

}
*/
void LogFile::WriteLog(const std::string &text)
{
	if(!m_fileptr)
	{
		OpenFile();	
	}
	
	if(m_fileptr)
	{
		if(m_writedate)
		{
			WriteDate();	
		}
		fputs(text.c_str(),m_fileptr);
		if(m_writenewline==true)
		{
			fputs("\r\n",m_fileptr);
		}
		fflush(m_fileptr);
	}
}
/*
void LogFile::WriteLog(const LogLevel level, const char *format, ...)
{
	if(level<=m_loglevel)
	{
		
		va_list va;
		va_start(va,format);
		
		if(!m_fileptr)
		{
			OpenFile();	
		}
		
		if(m_fileptr)
		{
			if(m_writedate)
			{
				WriteDate();	
			}
			if(m_writeloglevel)
			{
				WriteLogLevel(level);	
			}
			vfprintf(m_fileptr,format,va);
			if(m_writenewline==true)
			{
				fputs("\r\n",m_fileptr);
			}
			fflush(m_fileptr);
		}
		
		va_end(va);
	}
}
*/
void LogFile::WriteLog(const LogLevel level, const std::string &text)
{
	if(level<=m_loglevel)
	{
		if(!m_fileptr)
		{
			OpenFile();	
		}
		
		if(m_fileptr)
		{
			if(m_writedate)
			{
				WriteDate();	
			}
			if(m_writeloglevel)
			{
				WriteLogLevel(level);
			}
			fputs(text.c_str(),m_fileptr);
			if(m_writenewline==true)
			{
				fputs("\r\n",m_fileptr);
			}
			fflush(m_fileptr);
		}
	}	
}

void LogFile::WriteLogLevel(LogLevel level)
{
	if(m_fileptr)
	{
		switch(level)
		{
		case LOGLEVEL_FATAL:
			fputs("FATAL : ",m_fileptr);
			break;	
		case LOGLEVEL_ERROR:
			fputs("ERROR : ",m_fileptr);
			break;
		case LOGLEVEL_WARNING:
			fputs("WARN  : ",m_fileptr);
			break;
		case LOGLEVEL_INFO:
			fputs("INFO  : ",m_fileptr);
			break;
		case LOGLEVEL_DEBUG:
			fputs("DEBUG : ",m_fileptr);
			break;
		case LOGLEVEL_TRACE:
			fputs("TRACE : ",m_fileptr);
			break;
		default:
			fputs("????  : ",m_fileptr);
			break;
		}
	}
}

void LogFile::WriteNewLine()
{
	if(m_fileptr)
	{
		fputs("\r\n",m_fileptr);
		fflush(m_fileptr);
	}
}
