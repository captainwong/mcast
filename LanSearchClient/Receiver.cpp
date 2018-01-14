#include "Receiver.h"
#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>

#pragma comment(lib, "ws2_32.lib")

static bool multicastMembershipHelper(QUdpSocket* udpSocket,
									  int how4,
									  const QHostAddress &groupAddress,
									  const QNetworkInterface &iface)
{
	int level = 0;
	int sockOpt = 0;
	char *sockArg;
	int sockArgSize;

	struct ip_mreq mreq4;

	if (groupAddress.protocol() == QAbstractSocket::IPv4Protocol) {
		level = IPPROTO_IP;
		sockOpt = how4;
		sockArg = reinterpret_cast<char *>(&mreq4);
		sockArgSize = sizeof(mreq4);
		memset(&mreq4, 0, sizeof(mreq4));
		mreq4.imr_multiaddr.s_addr = htonl(groupAddress.toIPv4Address());

		if (iface.isValid()) {
			const QList<QNetworkAddressEntry> addressEntries = iface.addressEntries();
			bool found = false;
			for (const QNetworkAddressEntry &entry : addressEntries) {
				const QHostAddress ip = entry.ip();
				if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
					mreq4.imr_interface.s_addr = htonl(ip.toIPv4Address());
					found = true;
					break;
				}
			}
			if (!found) {
				return false;
			}
		} else {
			mreq4.imr_interface.s_addr = INADDR_ANY;
		}
	} else {
		return false;
	}

	int res = setsockopt(udpSocket->socketDescriptor(), level, sockOpt, sockArg, sockArgSize);
	if (res == -1) {
		return false;
	}
	return true;
}

Receiver::Receiver(QWidget *parent)
	: QDialog(parent)
{
	groupAddress = QHostAddress("239.255.43.21");
	auto localAddress = QHostAddress("192.168.1.222");

	statusLabel = new QLabel(tr("Listening for multicasted messages"));
	quitButton = new QPushButton(tr("&Quit"));

	auto udpSocket = new QUdpSocket(this);
	bool ret = udpSocket->bind(QHostAddress::AnyIPv4 /*localAddress*/, 45454, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	if (!ret) {
		qDebug() << "\t" << "bind result:" << ret << udpSocket->errorString();
		//continue;
	}

	//udpSocket->setSocketOption(QUdpSocket::MulticastLoopbackOption, "1");

	for (auto iface : QNetworkInterface::allInterfaces()) {
		if (iface.flags() & QNetworkInterface::CanMulticast 
			&& iface.flags() & QNetworkInterface::IsUp 
			&& iface.flags() & QNetworkInterface::IsRunning
			&& !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
			qDebug() << iface.name() << iface.humanReadableName() << iface.hardwareAddress();

			if (iface.humanReadableName() == "Local Area Connection"
				|| iface.humanReadableName() == QString::fromLocal8Bit("本地连接")) {

				for (auto addr : iface.addressEntries()) {
					if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
						qDebug() << "\t" << addr.ip().toString() << addr.netmask().toString() << addr.broadcast().toString();

						//ret = udpSocket->joinMulticastGroup(groupAddress, iface);
						/*ret = multicastMembershipHelper(udpSocket,
														IP_ADD_MEMBERSHIP,
														groupAddress,
														iface);*/

						ret = udpSocket->joinMulticastGroup(groupAddress, iface);
						qDebug() << "\t" << "join result:" << ret << udpSocket->errorString();
						if (!ret) {
							//qDebug() << "\t" << "join result:" << ret << udpSocket->errorString();
							continue;
						}

						connect(udpSocket, SIGNAL(readyRead()),
								this, SLOT(processPendingDatagrams()));
						udpSockets.push_back(udpSocket);
						break;
					}

				}

				/*ret = udpSocket->joinMulticastGroup(groupAddress, iface);
				qDebug() << "\t" << "join result:" << ret << udpSocket->errorString();
				if (!ret) {

					continue;
				}

				connect(udpSocket, SIGNAL(readyRead()),
						this, SLOT(processPendingDatagrams()));
				udpSockets.push_back(udpSocket);
				*/
				break;
			}
		}
	}

	//udpSocket->joinMulticastGroup(groupAddress);
	
	
	connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(quitButton);
	buttonLayout->addStretch(1);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(statusLabel);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	setWindowTitle(tr("Multicast Receiver"));
}

void Receiver::processPendingDatagrams()
{
	for (auto udpSocket : udpSockets) {
		while (udpSocket->hasPendingDatagrams()) {
			QByteArray datagram;
			datagram.resize(udpSocket->pendingDatagramSize());
			udpSocket->readDatagram(datagram.data(), datagram.size());
			auto msg = tr("Received datagram: \"%1\" from %2")
				.arg(datagram.data())
				.arg(udpSocket->peerAddress().toString());
			statusLabel->setText(msg);
			qDebug() << msg;
		}
	}
}

