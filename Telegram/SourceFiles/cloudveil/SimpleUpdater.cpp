#include "stdafx.h"
#include "SimpleUpdater.h"
#include "cloudveil/response/UpdateResponse.h"

//one day
#define UPDATE_PERIOD_MS 86400000 
#define REQUEST_URL "https://manage.cloudveil.org/api/v1/messenger/updates/windows?current_version=%1"

SimpleUpdater::SimpleUpdater(QObject *parent) : QObject(parent), manager(this)
{
	lastUpdateCheck = 0;
	versionNumber = AppVersion;
	connect(&timer, SIGNAL(timeout()), SLOT(doServerRequest()));
}


void SimpleUpdater::startUpdateChecking(int currentVersionNumber)
{
	this->versionNumber = currentVersionNumber;
	timer.stop();
	timer.setSingleShot(true);
	timer.start(2000);
}

void SimpleUpdater::doServerRequest() {
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - lastUpdateCheck < UPDATE_PERIOD_MS) {
		return;
	}

	QUrl url(QString(REQUEST_URL).arg(versionNumber));

	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));

	manager.get(request);
}


void SimpleUpdater::requestFinished(QNetworkReply *networkReply) {
	auto error = networkReply->error();

	if (error == QNetworkReply::NoError)
	{
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (httpStatusCode >= 200 && httpStatusCode < 300) // OK
		{
			QJsonDocument json = QJsonDocument::fromJson(networkReply->readAll());
			if (!json.isEmpty()) {
				QJsonObject jsonObj = json.object();
				lastResponse.readFromJson(jsonObj);
				
				if (lastResponse.url.size() > 0) {
					emit this->updateReceived(&lastResponse);
				}
			}
		}

		lastUpdateCheck = QDateTime::currentMSecsSinceEpoch();
	}
	else
	{
		qDebug() << "errorString: " << networkReply->errorString();
	}

	networkReply->deleteLater();
}

SimpleUpdater::~SimpleUpdater()
{
}
