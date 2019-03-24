#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"

namespace Config {
	int windowLen = 4; //窗口长度
	int maxSeqNum = 8;	//最大序号
}

TCPRdtSender::TCPRdtSender():base(1), nextSeqNum(1), N(Config::windowLen)
{
	timeoutCount = new int[Config::maxSeqNum];
	for (int i = 0; i < Config::maxSeqNum; i++) 
	{
		timeoutCount[i] = 0;
	}
}


TCPRdtSender::~TCPRdtSender()
{
	delete[]timeoutCount;
}



bool TCPRdtSender::getWaitingState() {
	/*return (nextSeqNum - base + Config::maxSeqNum) % Config::maxSeqNum >= N;*/
	return (nextSeqNum - base + Config::maxSeqNum) % Config::maxSeqNum >= N;
}


bool TCPRdtSender::send(Message &message) {
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


	pktQueue.push_back(sendPkt);	//放入缓存队列
	if (base == nextSeqNum) {
		timerSeq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);			//启动发送方定时器
	}
	pns->sendToNetworkLayer(RECEIVER, sendPkt);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	
	nextSeqNum = ++nextSeqNum % Config::maxSeqNum;
	return true;
}

//判断是否产生冗余
bool isDuplicate(int base, int N, int seq) {
	if (base < (base + N) % Config::maxSeqNum) {
		if (seq > base)
			return false;
		else
			return true;
	}
		
	else {
		if (seq <= base && seq > (base + N) % Config::maxSeqNum)
			return true;
		else
			return false;
	}
}

void TCPRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum) {
		pUtils->printPacket("正确接收到接收方发来的确认包：", ackPkt);
		if(!isDuplicate(base, N, ackPkt.acknum))
		{
			//未产生冗余，采用累计计数的条件下需要滑动窗口
			cout << "---》移动窗口前内容：\t";
			for (auto i : pktQueue)
			{
				cout << i.seqnum << " ";
			}
			cout << endl;

			for (int i = 0; i < (ackPkt.acknum - base + Config::maxSeqNum) % Config::maxSeqNum; i++)
				pktQueue.pop_front();
			base = (ackPkt.acknum) % Config::maxSeqNum;	
			if (base == nextSeqNum)
				pns->stopTimer(SENDER, timerSeq);		//关闭定时器
			else {
				pns->stopTimer(SENDER, timerSeq);
				timerSeq = base;
				pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);
			}

			cout << "---》移动窗口后内容：\t";
			for (auto i : pktQueue) {
				cout << i.seqnum << " ";
			}
			cout << endl;
		}
		else {
			timeoutCount[ackPkt.acknum]++;
			if (timeoutCount[ackPkt.acknum] == 3) {	//3个冗余ack
				cout << "3个冗余ack";
				for (auto i : pktQueue) {
					if (i.seqnum == ackPkt.acknum) {
						pns->sendToNetworkLayer(RECEIVER, i);//快速重传
						pUtils->printPacket("---》快速重传包：", i);
					}
				}
				timeoutCount[ackPkt.acknum] = 0;
			}
		}
	}
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	cout << "发生超时";
	if (base != nextSeqNum) { //窗口中有数据
		cout << "只重传最 早发送且没被确认的报文";
		pns->stopTimer(SENDER, seqNum);
		pns->sendToNetworkLayer(RECEIVER, pktQueue.front());	//后只重传最 早发送且没被确认的报文
		pns->startTimer(SENDER, Configuration::TIME_OUT, pktQueue.front().seqnum);
	}
	else {
		pns->stopTimer(SENDER, seqNum);
	}

}
