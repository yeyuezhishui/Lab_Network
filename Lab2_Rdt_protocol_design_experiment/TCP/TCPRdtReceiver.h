#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class TCPRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	TCPRdtReceiver();
	virtual ~TCPRdtReceiver();

public:
	
	void receive(Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

