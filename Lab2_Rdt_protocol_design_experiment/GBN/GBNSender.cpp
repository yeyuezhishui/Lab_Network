#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"
#include <cstring>

namespace Config
{
	int windowLen = 4;		//���ڳ���
	int maxseqnum = 8;		//������ ��ʾ��ֵΪ3λ��������
}

GBNRdtSender::GBNRdtSender():base(1), nextseqnum(1), N(Config::windowLen)
{
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return (nextseqnum-base+Config::maxseqnum)%Config::maxseqnum>=N;
}



/*�¼�1���߲����rdt����*/
bool GBNRdtSender::send(Message &message) {
	if (this->getWaitingState()) { //���Ķζ�������
		return false;
	}
	//����Ҫ���͵ı��Ķ�
	Packet &sendPkt = *new Packet;
	sendPkt.acknum = -1;
	sendPkt.seqnum = this->nextseqnum;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));		//���Ķ�����
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("GBNRdtSender���͵ı��ĶΣ�", sendPkt);

	pktQueue.push_back(sendPkt);	//���뵽�������ڶ��У���ʼ״̬Ϊ�����ã���δ���͡�


	if (base == nextseqnum)		//��ǰ������û�з��ͻ�δȷ�ϵģ�baseλ��ֱ���ǿ��û�δ���͵�
	{
		timerseq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);		//���ͷŶ�ʱ��
	}

	pns->sendToNetworkLayer(RECEIVER, sendPkt);		//���͵������

	nextseqnum = (nextseqnum + 1) % Config::maxseqnum;
	return true;
}

/*�¼�2���Ͳ���ý���*/
void GBNRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum)
	{
		pUtils->printPacket("��ȷ�յ�GBNRdtSender�����ı��ĶΣ�", ackPkt);
			cout << "---�ƶ���������ǰ���ݣ�\t";
			for (auto i : pktQueue)
			{
				cout << i.seqnum << " ";
			}
			cout << endl;
			for (int i = 0; i < (ackPkt.acknum - base+1+ Config::maxseqnum) % Config::maxseqnum; i++)	//GBN��ACK�����ۻ������ķ�ʽ���м���
			{
				pktQueue.erase(pktQueue.begin(), pktQueue.begin() + 1);
			}
			base = (ackPkt.acknum+1) % Config::maxseqnum;	//�ƶ�base
			cout << "---�ƶ����ں����ݣ�\t";
			for (auto i : pktQueue) {
				cout << i.seqnum << " ";
			}
			cout << endl;
		if (base == nextseqnum)
			pns->stopTimer(SENDER, timerseq);		//��ʱû���ѷ��͵���ȷ�ϵı��ĶΣ��رն�ʱ��
		else
		{
			pns->stopTimer(SENDER, timerseq);		//������ʱ��
			timerseq = base;		//���𵽱�ʶ����
			pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);
		}
	}
}

/*�¼�3����ʱ*/
void GBNRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);
	cout << "��ʱ";
	if (base != nextseqnum)	//��������֮ǰ�ѷ��ͣ���ʱ��Ҫ�ط�������
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
		for (auto i : pktQueue)
		{
			pns->sendToNetworkLayer(RECEIVER, i);
		}
	}
}
