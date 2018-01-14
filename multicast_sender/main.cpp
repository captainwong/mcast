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

int test_mcast_send()
{
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

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MULTICAST_GROUP_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	int addr_len = sizeof(addr);
	int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	if (-1 == ret) {
		printf("bind localaddr error!!!\n");
		perror("bind:");
		closesocket(sockfd);
		return -1;
	}

	unsigned long if_addr = inet_addr(LOCAL_IP);
	ret = setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&if_addr, sizeof(if_addr));
	if (-1 == ret) {
		printf("IP_MULTICAST_IF error!!!\n");
		perror("setsockopt:");
		closesocket(sockfd);
		return -1;
	}

	char host[1024] = { 0 };
	gethostname(host, 1024);

	int msgNo = 0;
	char msg[1024] = { 0 };
	addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP_ADDRESS);
	while (true) {
		//sprintf(msg, "Groupcast Message %s No.%d", host, msgNo++);
		sprintf(msg, "ControlCenterMessage %d", msgNo++);
		int ret = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)&addr, addr_len);
		if (ret < 0) {
			perror("sendto");
			return -1;
		} else {
			printf("Sent msg: %s\n", msg);
		}
		Sleep(1000);
	}

	return 0;
}

int main()
{
	int ret = init_winsock();
	if (0 != ret) { return ret; }

	ret = test_mcast_send();

	getchar();
}