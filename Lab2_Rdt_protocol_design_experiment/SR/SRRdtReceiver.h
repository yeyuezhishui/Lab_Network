#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include<utility>
typedef std::pair<Packet, bool> RecvPkt;

class SRRdtReceiver :public RdtReceiver
{
private:
	int base;				// ������ʼ
	int nextSeqNum;			// ��һ�����͵����
	const int N;			// ���ڴ�С
	RecvPkt *pktQueue;		//���洰�ڶ���

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:

	void receive(Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

