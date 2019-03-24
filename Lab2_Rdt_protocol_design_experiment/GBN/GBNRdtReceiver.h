#ifndef GBN_RDT_RECEIVER_H
#define GBN_RDT_RECEIVER_H
#include "RdtReceiver.h"
class GBNRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���,�е���tcp

public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:
	
	void receive(Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

