#include "stdafx.h"
#include "SimpleUpdater.h"
#include "cloudveil/response/UpdateResponse.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QProcess>

#include "ui/boxes/confirm_box.h"
//one day
#define UPDATE_PERIOD_MS 86400000 

#ifdef Q_OS_MAC
	#define REQUEST_URL "https://messenger.cloudveil.org/api/v1/messenger/updates/macos?current_version=%1"
#endif

#ifdef Q_OS_WIN
	#define REQUEST_URL "https://messenger.cloudveil.org/api/v1/messenger/updates/windows?current_version=%1"
#endif

#ifdef Q_OS_LINUX
	#define REQUEST_URL "https://messenger.cloudveil.org/api/v1/messenger/updates/linux?current_version=%1"
#endif

SimpleUpdater::SimpleUpdater(QObject *parent) : QObject(parent), manager(this)
{
	lastUpdateCheck = 0;
	versionNumber = AppVersion;

	connect(&timer, SIGNAL(timeout()), SLOT(doServerRequest()));
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestDone(QNetworkReply*)));
}


void SimpleUpdater::startUpdateChecking(int currentVersionNumber)
{
	this->versionNumber = currentVersionNumber;
	timer.stop();
	timer.setSingleShot(true);
	timer.start(2000);
}

void SimpleUpdater::downloadUpdate() {
	QNetworkRequest request(lastResponse.url);
	manager.get(request);
}

bool SimpleUpdater::startUpdateProcess() {
	QString filePath = downloadedFile.fileName();
	return QProcess::startDetached(filePath, QStringList());
}

void SimpleUpdater::doServerRequest() {
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - lastUpdateCheck < UPDATE_PERIOD_MS) {
		return;
	}

	QUrl url(QString(REQUEST_URL).arg(versionNumber));

	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");	
	manager.get(request);
}

void SimpleUpdater::networkRequestDone(QNetworkReply *networkReply) {
	bool isUpdateCheckResponse = networkReply->request().header(QNetworkRequest::ContentTypeHeader).toString().contains("/json");
	if (isUpdateCheckResponse) {
		processCheckResponse(networkReply);
	}
	else {
		processDownloadedResponse(networkReply);
	}
	
	networkReply->deleteLater();
}

void SimpleUpdater::processCheckResponse(QNetworkReply* networkReply) {
	auto error = networkReply->error();
	if (error == QNetworkReply::NoError) {
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (httpStatusCode >= 200 && httpStatusCode < 300) {
			QJsonDocument json = QJsonDocument::fromJson(networkReply->readAll());
			if (!json.isEmpty()) {
				QJsonObject jsonObj = json.object();
				lastResponse.readFromJson(jsonObj);

				if (lastResponse.url.size() > 0) {
					this->updateReceived(&lastResponse);
				}
			}
			lastUpdateCheck = QDateTime::currentMSecsSinceEpoch();
		}
	}
	else {
		qDebug() << "errorString: " << networkReply->errorString();
	}
}

void SimpleUpdater::processDownloadedResponse(QNetworkReply* networkReply) {
	auto error = networkReply->error();
	if (error == QNetworkReply::NoError) {
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (httpStatusCode >= 200 && httpStatusCode < 300) {

			QByteArray data = networkReply->readAll();
			if (downloadedFile.isOpen()) {
				downloadedFile.close();
			}
			
#ifdef Q_OS_WIN
			QString path = QDir::tempPath() + "/cvm_update_package.exe";
#elif defined(Q_OS_MAC)
			QString path = QDir::tempPath() + "/cvm_update_package.dmg";
#else
			QString path = QDir::tempPath() + "/cvm_update_package";
#endif
			downloadedFile.setFileName(path);
			if (QFile::exists(path)) {
				downloadedFile.remove();
			}
			if (downloadedFile.open(QIODevice::ReadWrite)) {
				if (data.size() != downloadedFile.write(data)) {
					downloadedFile.close();
					this->downloadFinished("Error writing update file.");
					qDebug() << "error: Can't write temp file";
				}
				else {
					downloadedFile.close();
					this->downloadFinished("");
				}
			}
			else {
				this->downloadFinished("Error writing update file.");
				qDebug() << "error: Can't save temp file";
			}
		}
		else {
			this->downloadFinished("Error downloading " + networkReply->errorString());
		}
	}
	else {
		this->downloadFinished("Error downloading " + networkReply->errorString());
		qDebug() << "errorString: " << networkReply->errorString();	
	}
}

SimpleUpdater::~SimpleUpdater()
{
}
