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
int nextAckNum;
int fileNameLen;
int main()
{
    init();
    while(1)
    {
        shakeHands();
        while(1)
        {
            memset(&receiveData,0,datagramLen);
            rd();
            checkSum(receiveData);
            if(receiveData.seqnum==nextAckNum)
            {
                //printf("receive data %d\n",(receiveData.seqnum-fileNameLen-1)/maxDataBufferSize);
                nextAckNum+=receiveData.dataLen;
                fwrite(receiveData.data,receiveData.dataLen,1,p);
                showDataInfo(receiveData);
                memset(&sendData,0,datagramHeaderLen);
                sendData.seqnum=1;
                sendData.ack=1;sendData.acknum=nextAckNum;
                sendData.rwnd=receiveData.rwnd;
                checkSum(sendData);
                if(receiveData.fin==1){
                    sendData.fin=1;
                    sd();
                    break;
                }
                sd();
            }
            else if(receiveData.seqnum>nextAckNum)
            {
                printf("unexpected data ");
                sd();
                showDataInfo(receiveData);
            }
            else if(receiveData.seqnum<nextAckNum)
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
    nextAckNum=1;
    //char fileName[50]="1.jpg";
    char fileName[50]="";
    char filePath[50]="";
    memset(&receiveData,0,datagramLen);
    rd();
    strcpy(fileName,receiveData.data);
    assert(receiveData.seqnum==nextAckNum);
    showDataInfo(receiveData);
    nextAckNum+=receiveData.dataLen;

    memset(&sendData,0,datagramHeaderLen);
    sendData.seqnum=1;
    sendData.ack=1;sendData.acknum=nextAckNum;
    sd();
    sprintf(filePath,"receive\\%s",fileName);
    if(!(p=fopen(filePath,"wb+")))
    {
        printf("create file error");
        printf("%s",filePath);
    }
    fileNameLen=strlen(fileName);
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