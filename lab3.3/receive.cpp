#include<winsock2.h>
#include<stdio.h>
#include<iostream>
#include<assert.h>
#include"myhead.h"
#include"receive.h"
#include<windows.h>
#pragma comment(lib,"ws2_32.lib")
datagram receiveData,sendData;
SOCKET serverSocket;
SOCKADDR_IN serverAddrInfo,sourceAddr;
FILE* p;
int tid;
const int port=6000;
int maxWindow=80000;
int nextAck;
int nextReceiveNum;
int fileNameLen;
int nextReadNum;
datagram buffer[dataBufferNum];
bool received[dataBufferNum];
int main()
{
    init();
    while(1)
    {
        shakeHands();
        CreateThread(NULL,NULL,&processDataThread,NULL,0,NULL);
        memset(received,0,sizeof(received));
        while(1)
        {
            memset(&receiveData,0,datagramLen);
            rd();
            checkSum(receiveData);
            if(receiveData.seqnum==nextAck)
            {
                memcpy(&buffer[nextReceiveNum],&receiveData,datagramLen);
                received[nextReceiveNum]=1;
                while(received[++nextReceiveNum]==1);
                datagram* d=&buffer[nextReceiveNum-1];
                nextAck=d->seqnum+d->dataLen;
                sendData.num=nextReceiveNum-1;
                sendData.rwnd=maxWindow-(nextReceiveNum-nextReadNum)*datagramLen;
                sendData.acknum=nextAck;sendData.ack=1;
                checkSum(sendData);
                if(receiveData.fin==1){
                    sendData.fin=1;
                    sd();
                    break;
                }
                sd();
                showDataInfo(receiveData);
            }
            else if(receiveData.seqnum>nextAck)//&&receiveData.seqNum<nextReadNum+maxWindow
            {
                received[receiveData.num]=1;
                memcpy(&buffer[receiveData.num],&receiveData,datagramLen);
                sd();
                printf("unexpected data");
                showDataInfo(receiveData);
            }
            else if(receiveData.seqnum<nextAck)
            {
                printf("duplicate data ");
                showDataInfo(receiveData);
            }
        }
        fclose(p);
        p=nullptr;
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
void shakeHands()
{
    memset(&receiveData,0,datagramLen);
    
    while(1){
        rd();
        if(receiveData.syn==1)
            break;
    };
    assert(receiveData.syn==1);
    showDataInfo(receiveData);
    
    memset(&sendData,0,datagramHeaderLen);
    sendData.ack=1;sendData.syn=1;sendData.acknum=1;
    sd();

    memset(&receiveData,0,datagramLen);
    rd();
    assert(receiveData.ack==1&&receiveData.acknum==1);
    showDataInfo(receiveData);
    //receivename
    nextAck=1;
    //char fileName[50]="1.jpg";
    char fileName[50]="";
    char filePath[50]="";
    memset(&receiveData,0,datagramLen);
    rd();
    strcpy(fileName,receiveData.data);
    assert(receiveData.seqnum==nextAck);
    showDataInfo(receiveData);
    nextAck+=receiveData.dataLen;

    memset(&sendData,0,datagramHeaderLen);
    sendData.seqnum=1;
    sendData.ack=1;sendData.acknum=nextAck;
    sd();
    sprintf(filePath,"receive\\%s",fileName);
    if(!(p=fopen(filePath,"wb+")))
    {
        printf("create file error");
        printf("%s",filePath);
    }
    fileNameLen=strlen(fileName);
}
DWORD WINAPI processDataThread(LPVOID lparam)
{
    while(1)
    {
        int temp=nextReceiveNum;
        if(nextReadNum<temp)
        {
            while(nextReadNum<temp)
            {
                fwrite(buffer[nextReadNum].data,buffer[nextReadNum].dataLen,1,p);
                nextReadNum++;
            }
        }
        else Sleep(100);
    }
}
void init()
{
    WSADATA wsaData;
    int err=WSAStartup(MAKEWORD(1,1),&wsaData);
    if(!err)printf("succeed to open receive socket\n");
    else printf("wrong \n");

    serverSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    serverAddrInfo.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    serverAddrInfo.sin_port=htons(port);
    serverAddrInfo.sin_family=AF_INET;

    if(SOCKET_ERROR==bind(serverSocket,(SOCKADDR*)&serverAddrInfo,sizeof(SOCKADDR)))
    {
        printf("fail to bind\n");
        closesocket(serverSocket);
    }
    else
        printf("succeed to bind\n");
}
void rd()
{
    if(SOCKET_ERROR==recvfrom(serverSocket,(char*)&receiveData,datagramLen,0,(SOCKADDR*)&sourceAddr,&addrLen))
    {
        printf("receive data error\n");
    }
}
void sd()
{
    if(SOCKET_ERROR==sendto(serverSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&sourceAddr,addrLen))
    {
        printf("send error\n");
    }
}