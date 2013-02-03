#include "logfile.h"

#include <ctime>
#include <cstdarg>
#include <cstdio>

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
	m_lastwriteday=-1;
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
	m_lastwriteday=-1;
}

LogFile::~LogFile()
{
	CloseFile();
	delete [] m_datebuffer;
}

void LogFile::CheckRotate()
{
	time_t rawtime=time(NULL);
	struct tm *timeinfo=gmtime(&rawtime);

	if(m_lastwriteday!=-1 && timeinfo->tm_mday!=m_lastwriteday)
	{
		Rotate(m_filename.substr(0,m_filename.find('.')),m_filename.substr(m_filename.find('.')+1));
	}

	m_lastwriteday=timeinfo->tm_mday;
}

bool LogFile::CloseFile()
{
	dlib::auto_mutex guard(m_mutex);
	if(m_fileptr)
	{
		fclose(m_fileptr);
		m_fileptr=NULL;
	}

	return true;
}

bool LogFile::OpenFile()
{
	dlib::auto_mutex guard(m_mutex);
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

void LogFile::Rotate(const std::string &filename, const std::string &extension, const int keep)
{
	CloseFile();

	std::string prevname("");
	for(int i=keep-1; i>=0; i--)
	{
		m_filename=filename;
		std::ostringstream ostr;
		ostr << i;
		if(i>0)
		{
			m_filename+="-"+ostr.str();
		}
		m_filename+="."+extension;

		if(i==keep-1)
		{
			unlink(m_filename.c_str());
		}
		else
		{
			rename(m_filename.c_str(),prevname.c_str());
		}

		prevname=m_filename;
	}

	OpenFile();

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

void LogFile::WriteLog(const std::string &text)
{
	dlib::auto_mutex guard(m_mutex);

	CheckRotate();

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

void LogFile::WriteLog(const LogLevel level, const std::string &text)
{
	dlib::auto_mutex guard(m_mutex);

	CheckRotate();

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
	dlib::auto_mutex guard(m_mutex);
	if(m_fileptr)
	{
		fputs("\r\n",m_fileptr);
		fflush(m_fileptr);
	}
}
