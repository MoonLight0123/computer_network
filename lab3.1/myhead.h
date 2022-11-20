#include<stdio.h>
#define timerId 100
const int maxBufferSize=4086;
const int timeInterval=200;
const int maxResend=5;
struct datagram
{
    int ack,syn,fin;
    unsigned short checksum;
    int seqnum,acknum;
    int dataLen;
    char data[maxBufferSize];
};
int addrLen=sizeof(SOCKADDR);
int datagramLen=sizeof(datagram);
int datagramHeaderLen=6*sizeof(int)+sizeof(short);
void showDataInfo(datagram& d)
{
    printf("ack:%d syn:%d fin:%d seqnum:%d acknum:%d checksum:%d datalen:%d\n",d.ack,d.syn,d.fin,d.seqnum,d.acknum,d.checksum,d.dataLen);
}
void checkSum(datagram &d)
{
    unsigned short* buff=(unsigned short*)&d;
    int count=datagramLen/sizeof(short);
    d.checksum=0;
    unsigned long sum=0;
    while(count--)
    {
        sum+=*buff++;
        if(sum&0xffff0000)
        {
            sum&=0xffff;
            sum++;
        }
    }
    d.checksum=~(sum&0xffff);
}