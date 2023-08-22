#pragma once
#include "cloudveil/response/UpdateResponse.h"
#include "mtproto/sender.h"
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTimer>

class SimpleUpdater : public QObject
{
	Q_OBJECT

public Q_SLOTS:
	void requestFinished(QNetworkReply *networkReply);

Q_SIGNALS:
	void updateReceived(UpdateResponse* response);

private:
	QTimer timer;
	int versionNumber;
	qint64 lastUpdateCheck;
	UpdateResponse lastResponse;
	QNetworkAccessManager manager;
	
private Q_SLOTS:
	void doServerRequest();

public:
	explicit SimpleUpdater(QObject *parent);
	
	void startUpdateChecking(int currentVersionNumber);

	~SimpleUpdater();
};

