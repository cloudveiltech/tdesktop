#pragma once
#include "cloudveil/response/UpdateResponse.h"

class SimpleUpdater : public QObject, public RPCSender
{
	Q_OBJECT

public slots:
	void requestFinished(QNetworkReply *networkReply);

signals:
	void updateReceived(UpdateResponse* response);

private:
	QTimer timer;
	int versionNumber;
	qint64 lastUpdateCheck;
	UpdateResponse lastResponse;
	QNetworkAccessManager manager;
	
private slots:
	void doServerRequest();

public:
	explicit SimpleUpdater(QObject *parent);
	
	void startUpdateChecking(int currentVersionNumber);

	~SimpleUpdater();
};

