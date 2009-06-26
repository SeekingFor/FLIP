/**

	\mainpage FCPv2 library

	FCPv2 C++ library
	
	link with ws2_32.lib and iphlpapi.lib in Windows

*/

/**
	\file fcpv2.h
*/

#ifndef _fcpv2_
#define _fcpv2_

#include <map>
#include <vector>
#include <string>

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

/**
	\brief %FCPv2 namespace

	This namespace contains the %FCPv2 library
*/
namespace FCPv2
{

/**
	\brief An FCP message

	FCP Messages are comprised of a name and zero or more field/value pairs.
*/
class Message
{
public:
	/**
		\brief Default constructor
	*/
	Message();
	/**
		\brief Construct message with a specific name

		\param name The name of the message
	*/
	Message(const std::string &name);
	/**
		\brief Construct message with a specific name and fields

		The number of field/value pairs must match the fieldcount parameter.

		\param name The name of the message
		\param fieldcount The number of field/value pairs that follow
	*/
	Message(const std::string &name, const int fieldcount, ...);

	/**
		\brief Gets the name of the message

		\return The name of the message
	*/
	const std::string &GetName() const							{ return m_name; }
	/**
		\brief Sets the name of the message

		\param name The name of the message
	*/
	void SetName(const std::string &name)						{ m_name=name; }

	/**
		\brief Accesses the field/value pairs

		\param field The field to access
		\return Reference to the value of the field
	*/
	std::string &operator[](const std::string &field)				{ return m_fields[field]; }

	/**
		\brief Non-const accessor for field map

		\return field map
	*/
	std::map<std::string,std::string> &GetFields()				{ return m_fields; }
	/**
		\brief Const accessor for field map

		\return field map
	*/
	const std::map<std::string,std::string> &GetFields() const	{ return m_fields; }
	
	/**
		\brief Clears the name and fields of the message
	*/
	void Clear()												{ m_name=""; m_fields.clear(); }
	
	const bool operator==(const Message &rhs) const				{ return (m_name==rhs.m_name && m_fields==rhs.m_fields); }
	const bool operator!=(const Message &rhs) const				{ return !(*this==rhs); }
	const bool operator<(const Message &rhs) const				{ return (m_name<rhs.m_name || (m_name==rhs.m_name && m_fields<rhs.m_fields)); }
	const bool operator<=(const Message &rhs) const				{ return (*this==rhs || *this<rhs); }
	const bool operator>(const Message &rhs) const				{ return !(*this<=rhs); }
	const bool operator>=(const Message &rhs) const				{ return !(*this<rhs); }
	
	/**
		\brief Gets the string representing this message in FCP

		This string is what is actually sent/received from the node through FCP

		\return The FCP message string
	*/
	const std::string GetFCPString() const;

private:

	std::string m_name;
	std::map<std::string,std::string> m_fields;

};

/**
	\brief An FCP connection to a Freenet node
*/
class Connection
{
public:
	/**
		\brief Default constructor
	*/
	Connection();
	/**
		\brief Construct connection with an existing socket

		\param sock An existing open socket connection to a Freenet node
	*/
	Connection(const int sock);
	/**
		\brief Default destructor

		The destructor will close the connection if it is open
	*/
	~Connection();

	/**
		\brief Creates an FCP connection to a Freenet node

		If the instanciated object has an existing connection open, it will be closed.

		\param fcphost The IP Address, hostname, FQDN, or other resolvable name that points to the Freenet node
		\param fcpport The port that the Freenet node is listening for FCP connections on
		\return true if the connection was established, false if it was not
	*/
	const bool Connect(const std::string &fcphost, const int fcpport);
	/**
		\brief Checks if the connection is currently connected

		\return true if there is a connection, false if there is not
	*/
	const bool IsConnected() const								{ return m_socket!=-1; }
	/**
		\brief Disconnects the connection

		\return always true
	*/
	const bool Disconnect();
	
	/**
		\brief Sends and receives data on the connection

		\param ms Maximum number of milliseconds to wait for the send and receive buffers to become available
		\return true if the connection remains connected, false if the connection is disconnected
	*/
	const bool Update(const unsigned long ms);

	/**
		\brief Checks if an FCP message is ready to be received

		\return true if an FCP message is ready to be received, false otherwise
	*/
	const bool MessageReady() const;
	
	/**
		\brief Gets the number of bytes on the receive buffer

		\return The number of bytes on the receive buffer
	*/
	const std::vector<char>::size_type ReceiveBufferSize() const	{ return m_receivebuffer.size(); }
	/**
		\brief Receives an FCP message
		
		\param[out] message The FCP message
		\return true if an FCP message was received, false otherwise
	*/
	const bool Receive(Message &message);
	/**
		\brief Receives raw data
		
		The received data is inserted at the end of the supplied vector

		\param[out] data vector to place received data in
		\param len number of bytes to receive
		\return true if the bytes were received, false otherwise
	*/
	const bool Receive(std::vector<char> &data, const std::vector<char>::size_type len);
	/**
		\brief Receives raw data

		\param[out] data char array to place received data in
		\param len number of bytes to receive
		\return true if the bytes were received, false otherwise
	*/
	const bool Receive(char *data, const size_t len);
	/**
		\brief Discards data on receive buffer

		\param len The number of bytes on the receive buffer to discard
		\return true if the bytes were discarded, false otherwise
	*/
	const bool ReceiveIgnore(const size_t len);
	
	/**
		\brief Gets the number of bytes waiting to be sent to the node

		\return The number of bytes waiting to be sent to the node
	*/
	const std::vector<char>::size_type SendBufferSize() const		{ return m_sendbuffer.size(); }
	/**
		\brief Sends an FCP Message

		\param message The Message to send
		\return true if the Message was buffered for sending successfully, false otherwise
	*/
	const bool Send(const Message &message);
	/**
		\brief Sends raw data

		\param data A vector of the data to send
		\return true if the data was buffered for sending successfully, false otherwise
	*/
	const bool Send(const std::vector<char> &data);
	/**
		\brief Sends raw data

		\param data A char array of data to send
		\param len The number of bytes on the array to send
		\return true if the data was buffered for sending successfully, false otherwise
	*/
	const bool Send(const char *data, const size_t len);

	/**
		\brief Gets the socket identifier of the connection

		\return The socket identifier.  It will be -1 if the socket is invalid.
	*/
	const int Socket()												{ return m_socket; }
	
	/**
		\brief Waits until the receive buffer contains a specified number of bytes
		
		This will continuously call Update until either the specific number of bytes have been received,
		or the connection becomes disconnected.
		
		\param ms The number of milliseconds for each call to Update
		\param len The number of bytes to wait for
		\return true if the number of bytes is waiting on the receive buffer, false if the connection was closed
	*/
	const bool WaitForBytes(const unsigned long ms, const size_t len);

private:
	// can't be copied
	Connection(const Connection &connection);
	Connection &operator=(const Connection &connection);

	const bool MessageReady(std::vector<char>::const_iterator &endpos, std::vector<char>::size_type &endlen) const;
	const bool MessageReady(std::vector<char>::iterator &endpos, std::vector<char>::size_type &endlen);
	void Split(const std::string &str, const std::string &separators, std::vector<std::string> &elements);

	void DoSend();
	void DoReceive();

#ifdef _WIN32
	static bool m_wsastartup;
#endif

	int m_socket;
	std::vector<char> m_receivebuffer;
	std::vector<char> m_sendbuffer;
	std::vector<char> m_tempbuffer;
	fd_set m_readfs;
	fd_set m_writefs;
	struct timeval m_timeval;
	
};

}	// namespace

#endif	// _fcpv2_connection_
