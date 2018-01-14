#pragma once

#include <QtWidgets/QDialog>
#include <QHostAddress>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QUdpSocket;
QT_END_NAMESPACE

class Receiver : public QDialog
{
	Q_OBJECT

public:
	Receiver(QWidget *parent = Q_NULLPTR);

private slots:
	void processPendingDatagrams();

private:
	QLabel *statusLabel;
	QPushButton *quitButton;
	QList<QUdpSocket*> udpSockets;
	QHostAddress groupAddress;

	

};
