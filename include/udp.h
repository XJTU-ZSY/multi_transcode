#pragma once
#include <windows.h>
#include"stdio.h"
class udp
{
public:
	udp();
	~udp();
	void init(int i);
	unsigned int server_socket_fd;
	unsigned int send_socket_fd;
	void sendinit(char* p,int i);
	sockaddr_in src_addr;
	sockaddr_in des_addr;

};