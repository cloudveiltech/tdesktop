#pragma once
#include "cloudveil/response/UpdateResponse.h"
#include "mtproto/sender.h"
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTimer>
#include <QtCore/QTemporaryFile>

class SimpleUpdater : public QObject
{
	Q_OBJECT

public Q_SLOTS:
	void networkRequestDone(QNetworkReply *networkReply);

Q_SIGNALS:
	void updateReceived(UpdateResponse* response);
	void downloadFinished(QString errorMessage);

private:
	QTimer timer;
	int versionNumber;
	qint64 lastUpdateCheck;
	UpdateResponse lastResponse;
	QNetworkAccessManager manager;
	QFile downloadedFile;
	
private:
	void processCheckResponse(QNetworkReply* networkReply);
	void processDownloadedResponse(QNetworkReply* networkReply);

private Q_SLOTS:
	void doServerRequest();

public:
	explicit SimpleUpdater(QObject *parent);
	
	void startUpdateChecking(int currentVersionNumber);
	void downloadUpdate();
	bool startUpdateProcess();

	~SimpleUpdater();
};

