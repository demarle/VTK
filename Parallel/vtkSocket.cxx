/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSocket.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSocket.h"

#include "vtkObjectFactory.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
  #define VTK_WINDOWS_FULL
  #include "vtkWindows.h"
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/time.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#define WSA_VERSION MAKEWORD(1,1)
#define vtkCloseSocketMacro(sock) (closesocket(sock))
#else
#define vtkCloseSocketMacro(sock) (close(sock))
#endif

vtkCxxRevisionMacro(vtkSocket, "1.1");
//-----------------------------------------------------------------------------
vtkSocket::vtkSocket()
{
  this->SocketDescriptor = -1;
  
}

//-----------------------------------------------------------------------------
vtkSocket::~vtkSocket()
{
  if (this->SocketDescriptor != -1)
    {
    this->CloseSocket(this->SocketDescriptor);
    this->SocketDescriptor = -1;
    }
}

//-----------------------------------------------------------------------------
int vtkSocket::CreateSocket()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  // Elimate windows 0.2 second delay sending (buffering) data.
  int on = 1;
  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on)))
    {
    return -1;
    }
  return sock;
}

//-----------------------------------------------------------------------------
int vtkSocket::BindSocket(int socketdescriptor, int port)
{
  struct sockaddr_in server;

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  // Allow the socket to be bound to an address that is already in use
  int opt=1;
#ifdef _WIN32
  setsockopt(socketdescriptor, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(int));
#else
  setsockopt(socketdescriptor, SOL_SOCKET, SO_REUSEADDR, (void *) &opt, sizeof(int));
#endif

  if ( bind(socketdescriptor, reinterpret_cast<sockaddr*>(&server), sizeof(server)) )
    {
    return -1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkSocket::Accept(int socketdescriptor)
{
  if (socketdescriptor < 0)
    {
    return -1;
    }
  return accept(socketdescriptor, 0, 0);
}

//-----------------------------------------------------------------------------
int vtkSocket::Listen(int socketdescriptor)
{
  if (socketdescriptor < 0)
    {
    return -1;
    }
  return listen(socketdescriptor, 1);
}

//-----------------------------------------------------------------------------
int vtkSocket::SelectSocket(int socketdescriptor, unsigned long msec)
{
  if (socketdescriptor < 0 )
    {
    // invalid socket descriptor.
    return -1;
    }
  
  fd_set rset;
  struct timeval tval;
  struct timeval* tvalptr = 0;
  if ( msec > 0 )
    {
    tval.tv_sec = msec / 1000;
    tval.tv_usec = (msec % 1000)*1000;
    tvalptr = &tval;
    }
  FD_ZERO(&rset);
  FD_SET(socketdescriptor, &rset);
  int res = select(socketdescriptor + 1, &rset, 0, 0, tvalptr);
  if(res == 0)
    {
    return 0;//for time limit expire
    }

  if ( res < 0 || !(FD_ISSET(socketdescriptor, &rset)) )
    {
    // Some error.
    return -1;
    }
  // The indicated socket has some activity on it.
  return 1;
}

//-----------------------------------------------------------------------------
int vtkSocket::SelectSockets(const int* sockets_to_select, int size,
    unsigned long msec, int* selected_index)
{
  int i;
  int max_fd = -1;
  *selected_index = -1;
  if (size <  0)
    {
    return -1;
    }
  
  fd_set rset;
  struct timeval tval;
  struct timeval* tvalptr = 0;
  if ( msec > 0 )
    {
    tval.tv_sec = msec / 1000;
    tval.tv_usec = msec % 1000;
    tvalptr = &tval;
    }
  FD_ZERO(&rset);
  for (i=0; i<size; i++)
    {
    FD_SET(sockets_to_select[i],&rset);
    max_fd = (sockets_to_select[i] > max_fd)? sockets_to_select[i] : max_fd;
    }
  
  int res = select(max_fd + 1, &rset, 0, 0, tvalptr);
  if (res == 0)
    {
    return 0; //Timeout
    }
  if (res < 0)
    {
    // SelectSocket error.
    return -1;
    }
  
  //check which socket has some activity.
  for (i=0; i<size; i++)
    {
    if ( FD_ISSET(sockets_to_select[i],&rset) )
      {
      *selected_index = i;
      return 1;
      }
    }
  return -1; 
}

//-----------------------------------------------------------------------------
int vtkSocket::Connect(int socketdescriptor, const char* hostName, int port)
{
  if (socketdescriptor < 0)
    {
    return -1;
    }

  struct hostent* hp;
  hp = gethostbyname(hostName);
  if (!hp)
    {
    unsigned long addr = inet_addr(hostName);
    hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    }

  if (!hp)
    {
    // vtkErrorMacro("Unknown host: " << hostName);
    return -1;
    }

  struct sockaddr_in name;
  name.sin_family = AF_INET;
  memcpy(&name.sin_addr, hp->h_addr, hp->h_length);
  name.sin_port = htons(port);

  return connect(socketdescriptor, reinterpret_cast<sockaddr*>(&name), 
    sizeof(name));
}

//-----------------------------------------------------------------------------
int vtkSocket::GetPort(int sock)
{
  struct sockaddr_in sockinfo;
  memset(&sockinfo, 0, sizeof(sockinfo));
#if defined(VTK_HAVE_GETSOCKNAME_WITH_SOCKLEN_T)
  socklen_t sizebuf = sizeof(sockinfo);
#else
  int sizebuf = sizeof(sockinfo);
#endif
  if(getsockname(sock, reinterpret_cast<sockaddr*>(&sockinfo), &sizebuf) != 0)
    {
    return 0;
    }
  return ntohs(sockinfo.sin_port);
}

//-----------------------------------------------------------------------------
void vtkSocket::CloseSocket(int socketdescriptor)
{
  if (socketdescriptor < 0)
    {
    return;
    }
  vtkCloseSocketMacro(socketdescriptor);
}

//-----------------------------------------------------------------------------
int vtkSocket::Send(void* data, int length)
{
  if (!this->GetConnected())
    {
    return 0;
    }
  if (length == 0)
    {
    // nothing to send.
    return 1;
    }
  char* buffer = reinterpret_cast<char*>(data);
  int total = 0;
  do
    {
    int n = send(this->SocketDescriptor, buffer+total, length-total, MSG_NOSIGNAL);
    if(n < 0)
      {
      vtkErrorMacro("Socket Error: Send failed.");
      return 0;
      }
    total += n;
    } while(total < length);
  return 1;
}

//-----------------------------------------------------------------------------
int vtkSocket::Receive(void* data, int length, int readFully/*=1*/)
{
  if (!this->GetConnected())
    {
    return 0;
    }

  char* buffer = reinterpret_cast<char*>(data);
  int total = 0;
  do
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
    int trys = 0;
#endif
    int n = recv(this->SocketDescriptor, buffer+total, length-total, 0);
    if(n < 1)
      {
#if defined(_WIN32) && !defined(__CYGWIN__)
      // On long messages, Windows recv sometimes fails with WSAENOBUFS, but
      // will work if you try again.
      int error = WSAGetLastError();
      if ((error == WSAENOBUFS) && (trys++ < 1000))
        {
        Sleep(1);
        continue;
        }
#endif
      vtkErrorMacro("Socket Error: Receive failed.");
      return 0;
      }
    total += n;
    } while(readFully && total < length);
  return total;
}

//-----------------------------------------------------------------------------
void vtkSocket::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SocketDescriptor: " << this->SocketDescriptor << endl;
}
