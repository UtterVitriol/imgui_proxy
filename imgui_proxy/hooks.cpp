#include "hooks.h"
#include "MinHook.h"

tWSARecv oWSARecv = NULL;
tWSASend oWSASend = NULL;

tRecv oRecv = NULL;
tSend oSend = NULL;

tRecvfrom oRecvfrom = NULL;
tSendto oSendto = NULL;

extern HookData g_hookData;

#define INT64_MAX_CHAR_LEN 20

Data data;
std::shared_ptr<MessageData> selected = NULL;

std::shared_ptr<MessageData> make_msg_data(int id, char* type, SOCKET sock, char* buf, int len)
{
	char temp[100] = { 0 };

	auto msg_data = std::make_shared<MessageData>();

	if (!msg_data)
	{
		exit(69);
	}

	msg_data->type.append(type);
	msg_data->type.append(":");
	sprintf_s(temp, 25, "%d", id);
	msg_data->type.append(temp);
	ZeroMemory(temp, 100);

	msg_data->sock = sock;

	sprintf_s(temp, 25, "%d", sock);
	msg_data->strSock.append(temp);
	ZeroMemory(temp, 100);

	msg_data->len = len;

	sprintf_s(temp, 25, "%d", len);
	msg_data->strLen.append(temp);

	for (int i = 0; i < len; i++)
	{
		sprintf_s(temp, 3, "%02hhX", buf[i]);
		msg_data->data.append(temp);
		if (i != len - 1)
		{
			msg_data->data.append(" ");
		}

		ZeroMemory(temp, 100);
	}

	return msg_data;
}

void send_modified()
{
	std::vector<char> out;
	char* end = (char*)selected->data.c_str();

	for (int i = 0; i < selected->len; i++)
	{
		out.push_back(strtol(end, &end, 16));
		if (end == NULL)
		{
			break;
		}
		printf("%02hhx", out[i]);
	}

	puts("");

	oSend(selected->sock, out.data(), selected->len, 0);

}

void make_message(SOCKET s, char* type, char* buf, int len)
{
	char temp[1000] = { 0 };

	std::string sTemp;

	sprintf_s(temp, INT64_MAX_CHAR_LEN, "%d", s);

	sTemp.append("[");
	sTemp.append(type);
	sTemp.append("-");
	sTemp.append(temp);

	ZeroMemory(temp, 100);
	sTemp.append(":");

	sprintf_s(temp, INT64_MAX_CHAR_LEN, "%d", len);

	sTemp.append(temp);

	ZeroMemory(temp, 100);

	sTemp.append("] ");

	for (int i = 0; i < len; i++)
	{
		sprintf_s(temp, 3, "%02hhX", buf[i]);
		sTemp.append(temp);
		sTemp.append(" ");
		ZeroMemory(temp, 100);
	}

	sTemp.append("\n");
	{
		std::unique_lock<std::mutex> lock(g_hookData.in_mutex);
		g_hookData.in.append(sTemp);
	}
}

int WINAPI hWSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	//g_hookData.in.append("wsaRecv\n");
	puts("wsaRecv");
	return oWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
}

int WINAPI hWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	//g_hookData.in.append("wsaSend\n");
	puts("wsaSend");
	return oWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, dwFlags, lpOverlapped, lpCompletionRoutine);
}

int WINAPI hrecv(SOCKET s, char* buf, int len, int flags)
{
	if (g_hookData.bRecv)
	{
		//make_message(s, (char*)"RECV", (char*)buf, len);
		{
			std::unique_lock<std::mutex> lock(data.mtx);
			data.data.push_back(make_msg_data(data.data.size(), (char*)"RECV", s, (char*)buf, len));
		}
	}

	//puts("recv");
	return oRecv(s, buf, len, flags);
}


int WINAPI hsend(SOCKET s, const char* buf, int len, int flags)
{
	if (g_hookData.bSend)
	{
		if (len > 0 and buf[0] != 0x63)
		{
			//make_message(s, (char*)"SEND", (char*)buf, len);
			{
				std::unique_lock<std::mutex> lock(data.mtx);
				data.data.push_back(make_msg_data(data.data.size(), (char*)"SEND", s, (char*)buf, len));
			}
		}
	}

	//puts("send");
	return oSend(s, buf, len, flags);
}

int WINAPI hrecvfrom(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen)
{
	puts("recvfrom");
	//g_hookData.in.append("Recvfrom\n");

	return oRecvfrom(s, buf, len, flags, from, fromlen);
}

int WINAPI hsendto(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
	puts("sendto");
	//g_hookData.in.append("sendto\n");

	return oSendto(s, buf, len, flags, to, tolen);
}

int init_hooks()
{
	int status = MH_OK;

	/*if ((status = MH_CreateHook(WSARecv, hWSARecv, (LPVOID*)&oWSARecv)) != MH_OK)
	{
		return status;
	}

	if ((status = MH_CreateHook(WSASend, hWSASend, (LPVOID*)&oWSASend)) != MH_OK)
	{
		return status;
	}*/

	if ((status = MH_CreateHook(recv, hrecv, (LPVOID*)&oRecv)) != MH_OK)
	{
		return status;
	}

	if ((status = MH_CreateHook(send, hsend, (LPVOID*)&oSend)) != MH_OK)
	{
		return status;
	}

	/*if ((status = MH_CreateHook(recvfrom, hrecvfrom, (LPVOID*)&oRecvfrom)) != MH_OK)
	{
		return status;
	}*/

	if ((status = MH_EnableHook(MH_ALL_HOOKS)) != MH_OK)
	{
		return status;
	}

	return status;
}