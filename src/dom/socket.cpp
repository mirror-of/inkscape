/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include <strings.h>
#include <stdio.h>

#include "socket.h"

//#########################################################################
//# U T I L I T Y
//#########################################################################

static void mybzero(void *s, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0)
        {
        *p++ = (unsigned char)0;
        n--;
        }
}

static void mybcopy(void *src, void *dest, size_t n)
{
    unsigned char *p = (unsigned char *)dest;
    unsigned char *q = (unsigned char *)src;
    while (n > 0)
        {
        *p++ = *q++;
        n--;
        }
}



//#########################################################################
//# T C P    C O N N E C T I O N
//#########################################################################

TcpSocket::TcpSocket()
{
    sock      = -1;
    hostname  = "";
    portno    = -1;
}


TcpSocket::TcpSocket(const char *hostnameArg, int port)
{
    sock      = -1;
    hostname  = hostnameArg;
    portno    = port;
}

TcpSocket::TcpSocket(const std::string &hostnameArg, int port)
{
    sock      = -1;
    hostname  = hostnameArg;
    portno    = port;
}



TcpSocket::TcpSocket(const TcpSocket &other)
{
    sock      = other.sock;
    hostname  = other.hostname;
    portno    = other.portno;
}

TcpSocket::~TcpSocket()
{
    disconnect();
}

bool TcpSocket::isOpen()
{
    return sock >= 0;
}


bool TcpSocket::connect(const char *hostnameArg, int portnoArg)
{
    hostname = hostnameArg;
    portno   = portnoArg;
    return connect();
}

bool TcpSocket::connect(const std::string &hostnameArg, int portnoArg)
{
    hostname = hostnameArg;
    portno   = portnoArg;
    return connect();
}


bool TcpSocket::connect()
{
    if (hostname.size()<1)
        {
        printf("open: null hostname\n");
        return false;
        }

    if (portno<1)
        {
        printf("open: bad port number\n");
        return false;
        }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        {
        printf("open: error creating socket\n");
        return false;
        }

    char *c_hostname = (char *)hostname.c_str();
    struct hostent *server = gethostbyname(c_hostname);
    if (!server)
        {
        printf("open: could not locate host '%s'\n", c_hostname);
        return false;
        }

    struct sockaddr_in serv_addr;
    mybzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    mybcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (::connect(sock, (const sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
        printf("open: could not connect to host '%s'\n", c_hostname);
        return false;
        }


    return true;
}

bool TcpSocket::disconnect()
{

    ::close(sock);
    sock = -1;

    return true;
}



bool TcpSocket::write(int ch)
{
    if (sock<0)
        {
        printf("write: socket closed\n");
        return false;
        }
    unsigned char c = (unsigned char)ch;

    if (send(sock, &c, 1, 0) < 0)
        {
        printf("write: could not send data\n");
        return false;
        }

    return true;
}

bool TcpSocket::write(char *str)
{
    if (!str)
        {
        return false;
        }

    printf("write:%s\n", str);

    while (*str)
        {
        if (!write((int)*str++))
            return false;
        }

    return true;
}

bool TcpSocket::write(const std::string &str)
{
    return write((char *)str.c_str());
}

int TcpSocket::read()
{
    if (!isOpen())
        {
        return -1;
        }

    unsigned char ch;
    if (recv(sock, &ch, 1, 0) <= 0)
        {
        disconnect();
        return -1;
        }

    return (int)ch;
}

std::string TcpSocket::readLine()
{
    std::string ret;

    while (isOpen())
        {
        int ch = read();
        if (ch<0)
            return ret;
        if (ch=='\r' || ch=='\n')
            return ret;
        ret.push_back((char)ch);
        }

    return ret;
}



static bool doTest()
{
    TcpSocket s;

    printf("### Connecting\n");
    if (!s.connect("www.mit.edu", 80))
        return false;
    printf("### Connected\n");

    if (!s.write("GET index.html HTTP/1.0\r\n\r\n"))
        return false;

    int ch = s.read();
    while (s.isOpen())
        {
        std::string str = s.readLine();
        printf("### %s\n", str.c_str());
        }


    s.disconnect();

    return true;
}






int main(int argc, char **argv)
{
    doTest();
    return 0;
}



//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
