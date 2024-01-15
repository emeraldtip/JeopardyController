#ifndef EE_NETWORKCUDPSOCKET_HPP
#define EE_NETWORKCUDPSOCKET_HPP

#include <eepp/network/ipaddress.hpp>
#include <eepp/network/socket.hpp>
#include <vector>

namespace EE { namespace Network {

class IpAddress;
class Packet;

/** @brief Specialized socket using the UDP protocol */
class EE_API UdpSocket : public Socket {
  public:
	// Constants
	enum {
		MaxDatagramSize =
			65507 ///< The maximum number of bytes that can be sent in a single UDP datagram
	};

	static UdpSocket* New();

	/** @brief Default constructor */
	UdpSocket();

	/** @brief Get the port to which the socket is bound locally
	**  If the socket is not bound to a port, this function
	**  returns 0.
	**  @return Port to which the socket is bound
	**  @see Bind */
	unsigned short getLocalPort() const;

	/** @brief Bind the socket to a specific port
	**  Binding the socket to a port is necessary for being
	**  able to receive data on that port.
	**  You can use the special value Socket::AnyPort to tell the
	**  system to automatically pick an available port, and then
	**  call GetLocalPort to retrieve the chosen port.
	**  @param port Port to Bind the socket to
	**  @param address Address of the interface to bind to
	**  @return Status code
	**  @see Unbind, GetLocalPort */
	Status bind( unsigned short port, const IpAddress& address = IpAddress::Any );

	/** @brief Unbind the socket from the local port to which it is bound
	**  The port that the socket was previously using is immediately
	**  available after this function is called. If the
	**  socket is not bound to a port, this function has no effect.
	**  @see Bind */
	void unbind();

	/** @brief Send raw data to a remote peer
	**  Make sure that @a size is not greater than
	**  UdpSocket::MaxDatagramSize, otherwise this function will
	**  fail and no data will be sent.
	**  @param data		  Pointer to the sequence of bytes to send
	**  @param size		  Number of bytes to send
	**  @param remoteAddress Address of the receiver
	**  @param remotePort	Port of the receiver to send the data to
	**  @return Status code
	**  @see Receive */
	Status send( const void* data, std::size_t size, const IpAddress& remoteAddress,
				 unsigned short remotePort );

	/** @brief Receive raw data from a remote peer
	**  In blocking mode, this function will wait until some
	**  bytes are actually received.
	**  Be careful to use a buffer which is large enough for
	**  the data that you intend to receive, if it is too small
	**  then an error will be returned and *all* the data will
	**  be lost.
	**  @param data		  Pointer to the array to fill with the received bytes
	**  @param size		  Maximum number of bytes that can be received
	**  @param received	  This variable is filled with the actual number of bytes received
	**  @param remoteAddress Address of the peer that sent the data
	**  @param remotePort	Port of the peer that sent the data
	**  @return Status code
	**  @see Send */
	Status receive( void* data, std::size_t size, std::size_t& received, IpAddress& remoteAddress,
					unsigned short& remotePort );

	/** @brief Send a formatted packet of data to a remote peer
	**  Make sure that the packet size is not greater than
	**  UdpSocket::MaxDatagramSize, otherwise this function will
	**  fail and no data will be sent.
	**  @param packet		Packet to send
	**  @param remoteAddress Address of the receiver
	**  @param remotePort	Port of the receiver to send the data to
	**  @return Status code
	**  @see Receive */
	Status send( Packet& packet, const IpAddress& remoteAddress, unsigned short remotePort );

	/** @brief Receive a formatted packet of data from a remote peer
	**  In blocking mode, this function will wait until the whole packet
	**  has been received.
	**  @param packet		Packet to fill with the received data
	**  @param remoteAddress Address of the peer that sent the data
	**  @param remotePort	Port of the peer that sent the data
	**  @return Status code
	**  @see Send */
	Status receive( Packet& packet, IpAddress& remoteAddress, unsigned short& remotePort );

	/** Set the send timeout. Only callable after bind ( after the socket
	 ** has been initialized ). */
	void setSendTimeout( SocketHandle sock, const Time& timeout );

	/** Set the receive timeout Only callable after bind ( after the socket
	 ** has been initialized ). */
	void setReceiveTimeout( SocketHandle sock, const Time& timeout );

  private:
	// Member data
	std::vector<char> mBuffer; ///< Temporary buffer holding the received data in Receive(Packet)
};

}} // namespace EE::Network

#endif // EE_NETWORKCUDPSOCKET_HPP

/**
@class EE::Network::UdpSocket

A UDP socket is a connectionless socket. Instead of
connecting once to a remote host, like TCP sockets,
it can send to and receive from any host at any time.

It is a datagram protocol: bounded blocks of data (datagrams)
are transfered over the network rather than a continuous
stream of data (TCP). Therefore, one call to send will always
match one call to receive (if the datagram is not lost),
with the same data that was sent.

The UDP protocol is lightweight but unreliable. Unreliable
means that datagrams may be duplicated, be lost or
arrive reordered. However, if a datagram arrives, its
data is guaranteed to be valid.

UDP is generally used for real-time communication
(audio or video streaming, real-time games, etc.) where
speed is crucial and lost data doesn't matter much.

Sending and receiving data can use either the low-level
or the high-level functions. The low-level functions
process a raw sequence of bytes, whereas the high-level
interface uses packets (see Packet), which are easier
to use and provide more safety regarding the data that is
exchanged. You can look at the Packet class to get
more details about how they work.

It is important to note that UdpSocket is unable to send
datagrams bigger than MaxDatagramSize. In this case, it
returns an error and doesn't send anything. This applies
to both raw data and packets. Indeed, even packets are
unable to split and recompose data, due to the unreliability
of the protocol (dropped, mixed or duplicated datagrams may
lead to a big mess when trying to recompose a packet).

If the socket is bound to a port, it is automatically
unbound from it when the socket is destroyed. However,
you can unbind the socket explicitely with the Unbind
function if necessary, to stop receiving messages or
make the port available for other sockets.

Usage example:
@code
// ----- The client -----

// Create a socket and bind it to the port 55001
UdpSocket socket;
socket.bind(55001);

// Send a message to 192.168.1.50 on port 55002
std::string message = "Hi, I am " + IpAddress::getLocalAddress().toString();
socket.Send(message.c_str(), message.size() + 1, "192.168.1.50", 55002);

// Receive an answer (most likely from 192.168.1.50, but could be anyone else)
char buffer[1024];
std::size_t received = 0;
IpAddress sender;
unsigned short port;
socket.receive(buffer, sizeof(buffer), received, sender, port);
std::cout << sender.toString() << " said: " << buffer << std::endl;

// ----- The server -----

// Create a socket and bind it to the port 55002
UdpSocket socket;
socket.bind(55002);

// Receive a message from anyone
char buffer[1024];
std::size_t received = 0;
IpAddress sender;
unsigned short port;
socket.receive(buffer, sizeof(buffer), received, sender, port);
std::cout << sender.toString() << " said: " << buffer << std::endl;

// Send an answer
std::string message = "Welcome " + sender.toString();
socket.send(message.c_str(), message.size() + 1, sender, port);
@endcode

@see EE::Network::Socket, EE::Network::TcpSocket, EE::Network::Packet
*/
