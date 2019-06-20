#include"udp.h"
udp::udp()
{
}
udp::~udp()
{

}
void udp::sendinit(char* p,int i)
{
	send_socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&des_addr, 0, sizeof(SOCKADDR_IN));
	des_addr.sin_addr.S_un.S_addr = inet_addr(p);
	des_addr.sin_family = AF_INET;
	//发送IP起始端口
	des_addr.sin_port = htons(10000 + i);
	bind(send_socket_fd, (LPSOCKADDR)&des_addr, sizeof(SOCKADDR_IN));
}

void udp::init(int i)
{
	
	server_socket_fd= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server_socket_fd < 0)
		printf("socket init error\n");
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.sin_family = AF_INET;
	src_addr.sin_addr.S_un.S_addr = inet_addr("1.8.86.30");
	src_addr.sin_port = htons(i+10020);
	int ret=0;
	ret = bind(server_socket_fd, (LPSOCKADDR)&src_addr, sizeof(src_addr));
	if (ret < 0)
		printf("bind error/n");

}