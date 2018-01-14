#include "Sender.h"
#include <QtNetwork>


Sender::Sender(QWidget *parent)
	: QDialog(parent)
{
	groupAddress = QHostAddress("239.255.43.21");

	statusLabel = new QLabel(tr("Ready to multicast datagrams to group %1 on port 45454").arg(groupAddress.toString()));

	ttlLabel = new QLabel(tr("TTL for multicast datagrams:"));
	ttlSpinBox = new QSpinBox;
	ttlSpinBox->setRange(0, 255);

	QHBoxLayout *ttlLayout = new QHBoxLayout;
	ttlLayout->addWidget(ttlLabel);
	ttlLayout->addWidget(ttlSpinBox);

	startButton = new QPushButton(tr("&Start"));
	quitButton = new QPushButton(tr("&Quit"));

	buttonBox = new QDialogButtonBox;
	buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

	timer = new QTimer(this);
	udpSocket = new QUdpSocket(this);
	//auto ret = udpSocket->joinMulticastGroup(groupAddress, QNetworkInterface::interfaceFromName("Local Area Connection"));
	//qDebug() << "\t" << "join result:" << ret << udpSocket->errorString();
	
	messageNo = 1;

	connect(ttlSpinBox, SIGNAL(valueChanged(int)), this, SLOT(ttlChanged(int)));
	connect(startButton, SIGNAL(clicked()), this, SLOT(startSending()));
	connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(timer, SIGNAL(timeout()), this, SLOT(sendDatagram()));

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(statusLabel);
	mainLayout->addLayout(ttlLayout);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);

	setWindowTitle(tr("Multicast Sender"));
	ttlSpinBox->setValue(1);
}

void Sender::ttlChanged(int newTtl)
{
	udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, newTtl);
}

void Sender::startSending()
{
	startButton->setEnabled(false);
	timer->start(1000);
}

void Sender::sendDatagram()
{
	statusLabel->setText(tr("Now sending datagram %1").arg(messageNo));
	QByteArray datagram = "Multicast message " + QByteArray::number(messageNo);
	udpSocket->writeDatagram(datagram.data(), datagram.size(),
							 groupAddress, 45454);
	++messageNo;
}