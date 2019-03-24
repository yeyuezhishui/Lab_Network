#include "stdafx.h"
#include "Global.h"
#include "SRRdtSender.h"

namespace SenderConfig {
	int windowLen = 4; //���ڳ���
	int maxSeqNum = 8;	//�����ţ� ��Ϊ��3λ���ֱ�ʾ
}

SRRdtSender::SRRdtSender() :base(0), nextSeqNum(0), N(SenderConfig::windowLen)
{
}


SRRdtSender::~SRRdtSender()
{
}



bool SRRdtSender::getWaitingState() {	
	return (nextSeqNum - base + SenderConfig::maxSeqNum) % SenderConfig::maxSeqNum >= N;
}


bool SRRdtSender::send(Message &message) {
	if (this->getWaitingState()) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	//����Ҫ���͵İ�
	Packet &sendPkt = *new Packet;
	sendPkt.acknum = -1; //���Ը��ֶ�
	sendPkt.seqnum = this->nextSeqNum;
	sendPkt.checksum = 0;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("���ͷ����ͱ���", sendPkt);
	pktQueue.push_back({ sendPkt, false });	//���봰�ڶ���


	timerSeq = sendPkt.seqnum;
	pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, sendPkt);								//ͨ������㷢�͵��Է�
	nextSeqNum = ++nextSeqNum % SenderConfig::maxSeqNum;
	return true;
}

void SRRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum && ackPkt.acknum != -1)
	{
		for (int i = 0; i < pktQueue.size(); i++)
		{
			if (pktQueue[i].first.seqnum == ackPkt.acknum) {
				pktQueue[i].second = true;	//��־�Ѿ��յ�
				pns->stopTimer(SENDER, pktQueue[i].first.seqnum); //�ر����Ķ�ʱ��
				pUtils->printPacket("���ͷ���ȷ����ȷ�ϱ���:", ackPkt);
			}
		}
		if (ackPkt.acknum == base) {

			cout << "---�����ͷ��ƶ�����ǰ���ݣ�\t";
			for (auto i : pktQueue) {
				cout << i.first.seqnum << " ";
			}
			cout << endl;
			int offset = 0;
			for (int i = 0; i < pktQueue.size(); i++) {
				if (pktQueue[i].second == true) {
					pns->stopTimer(SENDER, pktQueue[i].first.seqnum);	//�ر�������ļ�ʱ��
					base = (base + 1) % SenderConfig::maxSeqNum;
					offset++;
				}
				else
					break;
			}
			pktQueue.erase(pktQueue.begin(), pktQueue.begin() + offset);

			cout << "---�����ͷ��ƶ����ں����ݣ�\t";
			for (auto i : pktQueue) {
				cout << i.first.seqnum << " ";
			}
			cout << endl;
		}
	}
	else
		cout << "���ͷ����յ���ȷ�ϰ�У��Ͳ���ȷ" << checkSum << ackPkt.checksum << endl;
}

void SRRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);
	for (int i = 0; i < pktQueue.size(); i++) {
		if (pktQueue[i].first.seqnum == seqNum) {
			pns->sendToNetworkLayer(RECEIVER, pktQueue[i].first); //�ط������
		}
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);//������ʱ��
}
