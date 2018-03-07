#define REQUEST_URL "https://manage.cloudveil.org/api/v1/messenger/settings"

#include "stdafx.h"
#include "GlobalSecuritySettings.h"
#include "../mainwidget.h"
#include "../auth_session.h"
#include "../dialogs/dialogs_indexed_list.h"
#include "./request/SettingsRequest.h"
#include "./response/SettingsResponse.h"

SettingsResponse GlobalSecuritySettings::lastResponse;
GlobalSecuritySettings* GlobalSecuritySettings::instance;

bool GlobalSecuritySettings::loaded = false;


GlobalSecuritySettings::GlobalSecuritySettings(QObject *parent): QObject(parent), manager(this), timer(this) {
	lastResponse = SettingsResponse::loadFromCache();
	instance = this;
	connect(&timer, SIGNAL(timeout()), SLOT(doServerRequest()));
	loaded = true;
}

GlobalSecuritySettings::~GlobalSecuritySettings() {
	instance = nullptr;
}


void GlobalSecuritySettings::updateFromServer()
{
	timer.stop();
	timer.setSingleShot(true);
	timer.start(200);
}

void GlobalSecuritySettings::doServerRequest() {
	SettingsRequest request;
	buildRequest(request);
	sendRequest(request);
}

SettingsResponse& GlobalSecuritySettings::getSettings() {
	if (!loaded) {
		lastResponse = SettingsResponse::loadFromCache();
	}
	return lastResponse;
}


GlobalSecuritySettings* GlobalSecuritySettings::getInstance() {
	return instance;
}

void GlobalSecuritySettings::buildRequest(SettingsRequest &request) {
	Dialogs::IndexedList* dialogs = App::main()->dialogsList();
	for (auto i = dialogs->begin(); i != dialogs->end(); ++i) {
		PeerData* peer = (*i)->history()->peer;
		addDialogToRequest(request, peer);
	}

	request.userId = Auth().user()->bareId();
	request.userName = Auth().user()->username;
	request.userPhone = Auth().user()->phone();
}

void GlobalSecuritySettings::addDialogToRequest(SettingsRequest &request, PeerData *peer) {

	int32 dialogId = peer->bareId();
	SettingsRequest::Row row;
	row.id = dialogId;

	row.userName = peer->userName();

	if (peer->isChannel()) {
		row.title = peer->asChannel()->name;
		request.channels.append(row);
	}
	else if (peer->isChat()) {
		row.title = peer->asChat()->name;
		row.userName = row.title;
		request.groups.append(row);
	}
	else if (peer->isUser()) {
		if (peer->asUser()->botInfo.get() != nullptr) {
			row.title = peer->asUser()->name;
			request.bots.append(row);
		}
	}
}


void GlobalSecuritySettings::sendRequest(SettingsRequest &settingsRequestBody) {
	QUrl url(REQUEST_URL);
	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
	
	QJsonObject json;
	settingsRequestBody.writeToJson(json);

	QJsonDocument doc(json);

	qDebug() << "Sync" << doc.toJson(QJsonDocument::Compact);

	manager.post(request, doc.toJson(QJsonDocument::Compact));
}

void GlobalSecuritySettings::requestFinished(QNetworkReply *networkReply)
{
	// no error in request
	if (networkReply->error() == QNetworkReply::NoError)
	{
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (httpStatusCode >= 200 && httpStatusCode < 300) // OK
		{
			QJsonDocument json = QJsonDocument::fromJson(networkReply->readAll());
			if (!json.isEmpty()) {
				SettingsResponse settingsResponse;
				QJsonObject jsonObj = json.object();
				settingsResponse.readFromJson(jsonObj);
				settingsResponse.saveToCache();
				lastResponse = settingsResponse;

				emit this->settingsReady();
			}
		}
		else if (httpStatusCode >= 400 && httpStatusCode < 500) // 400 Error
		{
			qDebug() << httpStatusCode << " Error!";
		}
		else if (httpStatusCode >= 500 && httpStatusCode < 600) // 500 Internal Server Error
		{
			qDebug() << httpStatusCode << " Error!";
		}
		else
		{
			qDebug() << "Status code invalid! " << httpStatusCode;
		}
	}
	else
	{
		qDebug() << "errorString: " << networkReply->errorString();
	}

	networkReply->deleteLater();
}