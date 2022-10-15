#include<winsock2.h>
#include<stdio.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
const int port=6000;
struct param
{
    SOCKET s;
    SOCKADDR_IN addr;
    int id;
}p[5];
int num=0;
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
DWORD WINAPI createClient(LPVOID lparam)
{
    param* pp=(param*)lparam;
    SOCKET serverConnect=pp->s;
    int id=pp->id;
    int pid=id^1;
    //HANDLE hThread=CreateThread(NULL,NULL,&receiveMessage,&serverConnect,0,NULL);
    printf("client %d connect\n",id);
    char receiveBuffer[100];
    while(1)
    {
        int receiveLen=recv(serverConnect,receiveBuffer,100,0);
        if(receiveLen<0)
        {
            printf("fail to receive\n");
            break;
        }
        else 
        {
            printf("client %d send message to client %d: %s \n",id,pid,receiveBuffer);
            //printf("client %d socket %d,client %d socket %d\n",id,p[id].s,pid,p[pid].s);
            SOCKET serverConnect2=p[pid].s;
            int sendLen=send(serverConnect2,receiveBuffer,100,0);
            if(sendLen<0)
            {
                printf("server to client %d send error",pid);
            }
            if(receiveBuffer[0]=='e')
            {
                num--;
                printf("client %d exit\n",pid);
            }
        }
    }
}
int main()
{
    WSADATA wsaData;
    int err=WSAStartup(MAKEWORD(1,1),&wsaData);
    if(!err)printf("succeed to open server socket\n");
    else printf("wrong \n");
    
    SOCKET serverSocket=socket(AF_INET,SOCK_STREAM,0);

    SOCKADDR_IN addr;
    addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    addr.sin_family=AF_INET;

    int bindRes=bind(serverSocket,(SOCKADDR*)&addr,sizeof(SOCKADDR));
    if(bindRes==SOCKET_ERROR)printf("fail to bind\n");
    else printf("succeed to bind\n");

    int lisetenRes=listen(serverSocket,2);
    if(bindRes<0)printf("fail to listen\n");
    else printf("succeed to listen\n");
    int len=sizeof(SOCKADDR);
    while(1)
    {
        p[num].s=accept(serverSocket,(SOCKADDR*)&p[num].addr,&len);
        //printf("client 0 socket %d,client 1 socket %d,client 3 socket %d\n",p[0].s,p[1].s,p[2].s);
        SOCKET serverConnect=p[num].s;
        if(serverConnect==SOCKET_ERROR)
        {
            printf("fail to connect\n");
            WSACleanup();
            return 0;
        }

        char idBuffer[10];
        sprintf(idBuffer,"i:%d",num);
        send(serverConnect,idBuffer,10,0);
        p[num].id=num;
        HANDLE h=CreateThread(NULL,NULL,&createClient,&p[num],0,NULL);
        num++;
        CloseHandle(h);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}