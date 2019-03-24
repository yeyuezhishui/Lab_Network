#ifndef GBN_SENDER_H
#define GBN_SENDER_H
#include "RdtSender.h"
#include <vector>


class GBNRdtSender :public RdtSender
{
private:
	int base;		//������ʼ������δ��ȷ�ϻ����δ���͵ı��Ķ�
	int nextseqnum;			//����δ����
	int timerseq;		//timer�����
	const int N;		//���ڵĴ�С
	std::vector<Packet> pktQueue;		//�����еı��Ķζ���
public:

	bool getWaitingState();
	bool send(Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

