#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"
#include <cstring>

namespace Config
{
	int windowLen = 4;		//窗口长度
	int maxseqnum = 8;		//最大序号 表示该值为3位二进制数
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



/*事件1：高层调用rdt发送*/
bool GBNRdtSender::send(Message &message) {
	if (this->getWaitingState()) { //报文段队列已满
		return false;
	}
	//构造要发送的报文段
	Packet &sendPkt = *new Packet;
	sendPkt.acknum = -1;
	sendPkt.seqnum = this->nextseqnum;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));		//报文段数据
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("GBNRdtSender发送的报文段：", sendPkt);

	pktQueue.push_back(sendPkt);	//加入到滑动窗口队列，初始状态为“可用，还未发送”


	if (base == nextseqnum)		//当前队列中没有发送还未确认的，base位置直接是可用还未发送的
	{
		timerseq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);		//发送放定时器
	}

	pns->sendToNetworkLayer(RECEIVER, sendPkt);		//发送到网络层

	nextseqnum = (nextseqnum + 1) % Config::maxseqnum;
	return true;
}

/*事件2：低层调用接收*/
void GBNRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum)
	{
		pUtils->printPacket("正确收到GBNRdtSender发来的报文段：", ackPkt);
			cout << "---移动滑动窗口前内容：\t";
			for (auto i : pktQueue)
			{
				cout << i.seqnum << " ";
			}
			cout << endl;
			for (int i = 0; i < (ackPkt.acknum - base+1+ Config::maxseqnum) % Config::maxseqnum; i++)	//GBN的ACK采用累积计数的方式进行计数
			{
				pktQueue.erase(pktQueue.begin(), pktQueue.begin() + 1);
			}
			base = (ackPkt.acknum+1) % Config::maxseqnum;	//移动base
			cout << "---移动窗口后内容：\t";
			for (auto i : pktQueue) {
				cout << i.seqnum << " ";
			}
			cout << endl;
		if (base == nextseqnum)
			pns->stopTimer(SENDER, timerseq);		//此时没有已发送但待确认的报文段，关闭定时器
		else
		{
			pns->stopTimer(SENDER, timerseq);		//重启计时器
			timerseq = base;		//仅起到标识作用
			pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);
		}
	}
}

/*事件3：超时*/
void GBNRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);
	cout << "超时";
	if (base != nextseqnum)	//窗口中有之前已发送，此时需要重发的数据
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
		for (auto i : pktQueue)
		{
			pns->sendToNetworkLayer(RECEIVER, i);
		}
	}
}
