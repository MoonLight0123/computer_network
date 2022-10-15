#include<winsock2.h>
#include<stdio.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
const char ip[15]="127.0.0.1";
const int port=6000;
int id;
void getTime(char* buff)
{
    time_t mytime;
	time(&mytime);
	mytime = time(NULL); 
	time_t PTime = 0;
    time_t time = mytime;
    struct tm* timeP;
    char buffer[128];
    PTime = time;
    timeP = localtime(&PTime);
	sprintf(buff,"%d/%d/%d %d:%d:%d:",1900+ timeP->tm_year,1+ timeP->tm_mon,timeP->tm_mday, timeP->tm_hour, timeP->tm_min, timeP->tm_sec);
}
DWORD WINAPI receiveMessage(LPVOID lparam)
{
    SOCKET clientSocket=*((SOCKET*)lparam);
    char receiveBuffer[100];
    while(1)
    {
        int receiveLen=recv(clientSocket,receiveBuffer,100,0);
        if(receiveLen<0)
        {
            printf("fail to receive\n");
            break;
        }
        else if(receiveBuffer[0]=='i')
        {
            id=receiveBuffer[2]-'0';
            printf("welocme! client %d\n",id);
        }
        else if(receiveBuffer[0]=='m')
        {
            printf("client %d message: %s \n",id^1,receiveBuffer+2);
        }
        else if(receiveBuffer[0]=='e')
        {
            printf("client %d exit\n",id^1);
        }
    }
}
int main()
{
    WSADATA wsaData;
    int err;
    err=WSAStartup(MAKEWORD(1,1),&wsaData);
    if(!err)printf("succeed to open client socket\n");
    else printf("wrong \n");

    SOCKET clientSocket=socket(AF_INET,SOCK_STREAM,0);

    SOCKADDR_IN clientSocket_in;//server address info
    clientSocket_in.sin_addr.S_un.S_addr=inet_addr(ip);
    clientSocket_in.sin_family=AF_INET;
    clientSocket_in.sin_port=htons(port);


    if(SOCKET_ERROR==connect(clientSocket,(SOCKADDR*)&clientSocket_in,sizeof(SOCKADDR)))
    {
        printf("fail to connect\n");
        WSACleanup();
        return 0;
    }
    else printf("succeed to connect\n");
    
    
    
    HANDLE hThread=CreateThread(NULL,NULL,&receiveMessage,&clientSocket,0,NULL);
    char scanfBuffer[100];
    char sendBuffer[100];
    char timeBuffer[100];
    while(1)
    {
        scanf("%s",&scanfBuffer);

        if(strcmp(scanfBuffer,"exit"))
        {
            getTime(timeBuffer);
            sprintf(sendBuffer,"m:%s %s",timeBuffer,scanfBuffer);
        }
        else sprintf(sendBuffer,"e:");
            
        
        int sendLen=send(clientSocket,sendBuffer,100,0);
        if(sendLen<0)
        {
            printf("fail to send\n");
            break;
        }
        else if(sendBuffer[0]=='m')printf("successfully send\n");
        else if(sendBuffer[0]=='e')break;
    }
    CloseHandle(hThread);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}