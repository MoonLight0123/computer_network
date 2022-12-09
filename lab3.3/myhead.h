#include<stdio.h>
const int maxDataBufferSize=1024;
const int timeInterval=300;
const int maxResend=1;
const int dataBufferNum=15000;
struct datagram
{
    unsigned short ack,syn,fin;
    unsigned short checksum;
    unsigned short num;
    int seqnum,acknum;
    int dataLen; 
    int rwnd;
    char data[maxDataBufferSize];
};
int addrLen=sizeof(SOCKADDR);
int datagramLen=sizeof(datagram);
int datagramHeaderLen=datagramLen-maxDataBufferSize;
void showDataInfo(datagram& d)
{
    printf("datanum:%d ack:%d syn:%d fin:%d seqnum:%d acknum:%d checksum:%d datalen:%d rwnd:%d\n",d.num,d.ack,d.syn,d.fin,d.seqnum,d.acknum,d.checksum,d.dataLen,d.rwnd);
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