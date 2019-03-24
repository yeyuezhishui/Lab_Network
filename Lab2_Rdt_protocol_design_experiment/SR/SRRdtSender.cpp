#include "stdafx.h"
#include "Global.h"
#include "SRRdtSender.h"

namespace SenderConfig {
	int windowLen = 4; //窗口长度
	int maxSeqNum = 8;	//最大序号， 因为是3位数字表示
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
	if (this->getWaitingState()) { //发送方处于等待确认状态
		return false;
	}

	//构造要发送的包
	Packet &sendPkt = *new Packet;
	sendPkt.acknum = -1; //忽略该字段
	sendPkt.seqnum = this->nextSeqNum;
	sendPkt.checksum = 0;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("发送方发送报文", sendPkt);
	pktQueue.push_back({ sendPkt, false });	//放入窗口队列


	timerSeq = sendPkt.seqnum;
	pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, sendPkt);								//通过网络层发送到对方
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
				pktQueue[i].second = true;	//标志已经收到
				pns->stopTimer(SENDER, pktQueue[i].first.seqnum); //关闭它的定时器
				pUtils->printPacket("发送方正确接收确认报文:", ackPkt);
			}
		}
		if (ackPkt.acknum == base) {

			cout << "---》发送方移动窗口前内容：\t";
			for (auto i : pktQueue) {
				cout << i.first.seqnum << " ";
			}
			cout << endl;
			int offset = 0;
			for (int i = 0; i < pktQueue.size(); i++) {
				if (pktQueue[i].second == true) {
					pns->stopTimer(SENDER, pktQueue[i].first.seqnum);	//关闭这个包的计时器
					base = (base + 1) % SenderConfig::maxSeqNum;
					offset++;
				}
				else
					break;
			}
			pktQueue.erase(pktQueue.begin(), pktQueue.begin() + offset);

			cout << "---》发送方移动窗口后内容：\t";
			for (auto i : pktQueue) {
				cout << i.first.seqnum << " ";
			}
			cout << endl;
		}
	}
	else
		cout << "发送方接收到的确认包校验和不正确" << checkSum << ackPkt.checksum << endl;
}

void SRRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);
	for (int i = 0; i < pktQueue.size(); i++) {
		if (pktQueue[i].first.seqnum == seqNum) {
			pns->sendToNetworkLayer(RECEIVER, pktQueue[i].first); //重发这个包
		}
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);//重启计时器
}
