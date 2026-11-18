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
	bool fail(const QString &error);
#if !defined Q_OS_WIN && !defined Q_OS_MAC
	bool installLinuxUpdate(const QString &archivePath);
#endif
	QString lastError;

private Q_SLOTS:
	void doServerRequest();

public:
	explicit SimpleUpdater(QObject *parent);
	
	void startUpdateChecking(int currentVersionNumber);
	void downloadUpdate();
	bool startUpdateProcess();
	// Why the last startUpdateProcess() failed.
	QString errorMessage() const { return lastError; }
	// True when the app should restart itself (the update replaced it in
	// place) rather than quit and let an installer take over.
	static constexpr bool kRestartsInPlace =
#if !defined Q_OS_WIN && !defined Q_OS_MAC
		true;
#else
		false;
#endif

	~SimpleUpdater();
};

