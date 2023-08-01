#pragma once

#include <WinSock2.h>
#include <string>
#include <mutex>
#include <vector>
#include <memory>
#pragma comment(lib, "Ws2_32.lib")

class HookData
{
public:
	std::string in;
	std::mutex in_mutex;
	std::string out;
	bool bSend = false;
	bool bRecv = false;
};

typedef class MessageData_
{
public:
	std::string type;
	SOCKET sock = 0;
	std::string strSock;
	int len = 0;
	std::string strLen;
	std::string data;
}MessageData;


class Data
{
public:
	std::vector<std::shared_ptr<MessageData>> data;
	std::mutex mtx;
};


typedef int (WINAPI *tWSARecv)(SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
typedef int (WINAPI *tWSASend)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);

typedef int (WINAPI *tSend)(SOCKET, const char*, int, int);
typedef int (WINAPI *tRecv)(SOCKET, char*, int, int);

typedef int (WINAPI *tRecvfrom)(SOCKET, char*, int, int, sockaddr*, int*);
typedef int (WINAPI *tSendto)(SOCKET, const char*, int, int, const sockaddr*, int);

int init_hooks();
void send_modified();
