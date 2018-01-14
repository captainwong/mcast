#pragma once

#include <QtWidgets>
#include <QHostAddress>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QPushButton;
class QTimer;
class QUdpSocket;
class QSpinBox;
QT_END_NAMESPACE

class Sender : public QDialog
{
	Q_OBJECT

public:
	Sender(QWidget *parent = nullptr);
	~Sender() {}

private slots:
	void ttlChanged(int newTtl);
	void startSending();
	void sendDatagram();

private:
	QLabel *statusLabel;
	QLabel *ttlLabel;
	QSpinBox *ttlSpinBox;
	QPushButton *startButton;
	QPushButton *quitButton;
	QDialogButtonBox *buttonBox;
	QUdpSocket *udpSocket;
	QTimer *timer;
	QHostAddress groupAddress;
	int messageNo;

};
