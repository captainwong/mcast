//#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

static const auto LOCAL_IP = "192.168.1.222";
static const auto MULTICAST_GROUP_ADDRESS = "239.255.43.21";
static const unsigned short MULTICAST_GROUP_PORT = 45454;
static const int MSGBUFSIZE = 1024;

int init_winsock()
{
	int iResult;
	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	return 0;
}

int test_multicast_with_local_ip()
{
	int ret = 0;

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd) {
		printf("socket error!!!\n");
		perror("socket:");
		return -1;
	}

	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0) {
		perror("Setting SO_REUSEADDR error");
		closesocket(sockfd);
		return -1;
	}

	/*test ip*/
	struct sockaddr_in localaddr = { 0 };
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(MULTICAST_GROUP_PORT);
	localaddr.sin_addr.s_addr = /*inet_addr(LOCAL_IP)*/ htonl(INADDR_ANY);
	ret = bind(sockfd, (struct sockaddr*)&localaddr, sizeof(struct sockaddr));
	if (-1 == ret) {
		printf("bind localaddr error!!!\n");
		perror("bind:");
		closesocket(sockfd);
		return -1;
	}

	

	/*设置是否支持本地回环接收*/
	/*int loopBack = 1;
	ret = setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&loopBack, sizeof(loopBack));
	if (-1 == ret) {
	printf("setsockopt broadcaset error!!!\n");
	perror("setsockopt:");
	closesocket(sockfd);
	return -1;
	}*/

	/*
	将本地socket添加到多播组中，注意，此处针对struct ip_mreq结构体需要填充两个成员，
	成员ipmr.imr_interface.s_addr的值指定的是将要发送的网卡的ip地址，
	成员impr.imr_multiaddr指定的是组播地址；
	如果指定为INADDR_ANY则系统会绑定一个默认网卡的具体ip（根据默认网关选择），则会出现特定网卡可以发送和接收组播信息，另一网卡不可以。
	即指定INADDR_ANY并不能把所有网卡都添加多播组中，必须明确指定对应网卡ip才可以。*/
	//struct in_addr addr = { 0 };
	//addr.s_addr = inet_addr(local_ip);
	struct ip_mreq ipmr = { 0 };
	ipmr.imr_interface.s_addr = inet_addr(LOCAL_IP) /*(INADDR_ANY)*/;
	ipmr.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP_ADDRESS);
	int len = sizeof(ipmr);
	ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipmr, len);
	if (-1 == ret) {
		printf("set error IP_ADD_MEMBERSHIP %d\n", WSAGetLastError());
		perror("setsockopt:");
		closesocket(sockfd);
		return -1;
	}

	/*此处指定组播数据的出口网卡，如果不设置则会根据路由表指定默认路由出口*/
	/*if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (const char *)&addr, sizeof(addr))) {
		printf("set error IP_MULTICAST_IF %s\n", local_ip);
		perror("Setting IP_MULTICAST_IF error:");
		closesocket(sockfd);
		sockfd = -1;
	}*/

	/*struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	if (-1 == ret) {
		printf("setsockopt recvtimeout error!!!\n");
		perror("setsockopt:");
		closesocket(sockfd);
		return -1;
	}*/

	/* now just enter a read-print loop */
	char msgbuf[MSGBUFSIZE];
	int nbytes = 0;
	localaddr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP_ADDRESS);
	while (1) {
		int addrlen = sizeof(localaddr);
		if ((nbytes = recvfrom(sockfd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &localaddr, &addrlen)) < 0) {
			perror("recvfrom");
			return -1;
		}
		msgbuf[nbytes] = 0;
		puts(msgbuf);
	}

	return 0;
}

int test_multicast()
{
	int fd;
	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return(1);
	}

	/**** MODIFICATION TO ORIGINAL */
	/* allow multiple sockets to use the same PORT number */
	u_int yes = 1;            /*** MODIFICATION TO ORIGINAL */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		return(1);
	}
	/*** END OF MODIFICATION TO ORIGINAL */

	/* set up destination address */
	struct sockaddr_in addr = {};
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
	addr.sin_port = htons(MULTICAST_GROUP_PORT);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		return(1);
	}

	/* use setsockopt() to request that the kernel join a multicast group */
	struct ip_mreq mreq = {};
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP_ADDRESS);
	//mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) < 0) {
		perror("IP_ADD_MEMBERSHIP");
		//return(1);
	}

	/* now just enter a read-print loop */
	int nbytes = 0;
	int addrlen = sizeof(addr);
	char msgbuf[MSGBUFSIZE] = {};
	while (1) {
		if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, &addrlen)) < 0) {
			perror("recvfrom");
			return(1);
		}
		msgbuf[nbytes] = 0;
		printf("%s\n", msgbuf);
	}

	return 0;
}

int main()
{
	int ret = init_winsock();
	if (0 != ret) { return ret; }

	ret = test_multicast_with_local_ip();

	//ret = test_multicast();

	getchar();
}