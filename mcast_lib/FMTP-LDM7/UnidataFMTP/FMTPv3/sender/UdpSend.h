/**
 * Copyright (C) 2014 University of Virginia. All rights reserved.
 *
 * @file      UdpSend.h
 * @author    Shawn Chen <sc7cq@virginia.edu>
 * @version   1.0
 * @date      Oct 23, 2014
 *
 * @section   LICENSE
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @brief     Define the interfaces and structures of sender side UDP layer
 *            abstracted funtions.
 *
 * The UdpSend class includes a set of transmission functions, which are
 * basically the encapsulation of udp system calls themselves. This abstracted
 * new layer acts as the sender side transmission library.
 */


#ifndef FMTP_SENDER_UDPSOCKET_H_
#define FMTP_SENDER_UDPSOCKET_H_

#include "fmtpBase.h"
#include "HmacImpl.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>


class UdpSend
{
public:
    UdpSend(const std::string& recvaddr, const unsigned short recvport,
            const unsigned char ttl, const std::string& ifAddr);

    ~UdpSend();

    /**
     *
     * Initializes this instance. Creates a new UDP socket and sets the address
     * and port from the construction parameters. Connects to the created
     * socket.
     */
    void Init();

    /**
     * Returns the key used to compute the message authentication code of FMTP
     * messages.
     *
     * @return  Key used to compute the MAC of FMTP messages
     */
    const std::string& getMacKey() const noexcept;

    /**
     * Sends an FMTP message. The FMTP header is sent in network byte-order. The
     * payload is sent as-is (i.e., it is not converted to network byte-order).
     *
     * @param[in] header         FMTP header in *host* byte-order
     * @param[in] payload        FMTP message payload. Size, in bytes, is given
     *                           by `header.payloadlen`. Ignored if size is
     *                           zero.
     * @throw std::logic_error   `header.payloadlen && payload == nullptr`
     * @throw std::system_error  I/O failure
     */
    void send(const FmtpHeader& header,
    		  const void*       payload = nullptr);

private:
    int                   sock_fd;
    struct sockaddr_in    recv_addr;
    const std::string     recvAddr;
    const unsigned short  recvPort;
    const unsigned short  ttl;
    const std::string     ifAddr;
    HmacImpl              hmacImpl;
	FmtpHeader            netHead;       ///< Network byte-order FMTP header
    struct iovec          iov[3];        ///< Output vector
    char                  mac[MAC_SIZE]; ///< Message authentication code
};


#endif /* FMTP_SENDER_UDPSOCKET_H_ */
