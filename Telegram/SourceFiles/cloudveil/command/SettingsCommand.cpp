#define REQUEST_URL "https://manage.cloudveil.org/api/v1/messenger/settings"

#include "stdafx.h"
#include "SettingsCommand.h"
#include "../../mainwidget.h"
#include "../../auth_session.h"
#include "../../dialogs/dialogs_indexed_list.h"
#include "../request/SettingsRequest.h"
#include "../response/SettingsResponse.h"

SettingsCommand::SettingsCommand(QObject *parent): QObject(parent) {

}

void SettingsCommand::run()
{
	SettingsRequest request;	
	buildRequest(request);
	sendRequest(request);	
}

void SettingsCommand::buildRequest(SettingsRequest &request) {
	Dialogs::IndexedList* dialogs = App::main()->dialogsList();
	for (auto i = dialogs->begin(); i != dialogs->end(); ++i) {
		PeerData* peer = (*i)->history()->peer;

		int32 dialogId = peer->bareId();
		SettingsRequest::Row row;
		row.id = dialogId;

		if (peer->isChannel()) {
			row.title = peer->asChannel()->name;
			row.userName = peer->asChannel()->userName();
			request.channels.append(row);
		}
		else if (peer->isChat()) {
			row.title = peer->asChat()->name;
			row.userName = peer->asChat()->userName();
			request.groups.append(row);
		}
		else if (peer->isUser()) {
			if (peer->asUser()->botInfo.get() != nullptr) {
				row.title = peer->asUser()->name;
				row.userName = peer->asUser()->username;
				request.bots.append(row);
			}
		}
	}

	request.userId = Auth().user()->bareId();
	request.userName = Auth().user()->username;
	request.userPhone = Auth().user()->phone();
}

void SettingsCommand::sendRequest(SettingsRequest &settingsRequestBody) {
	QUrl url(REQUEST_URL);
	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
	
	QJsonObject json;
	settingsRequestBody.writeToJson(json);

	QJsonDocument doc(json);

	// FIXME for debug
	qDebug() << "Sync" << doc.toJson(QJsonDocument::Compact);

	manager.post(request, doc.toJson(QJsonDocument::Compact));
}

void SettingsCommand::requestFinished(QNetworkReply *networkReply)
{
	// free later
	networkReply->deleteLater();

	// no error in request
	if (networkReply->error() == QNetworkReply::NoError)
	{
		// get HTTP status code
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		// 200
		if (httpStatusCode >= 200 && httpStatusCode < 300) // OK
		{
			QJsonDocument json = QJsonDocument::fromJson(networkReply->readAll());
			SettingsResponse settingsResponse;
			QJsonObject jsonObj = json.object();
			settingsResponse.readFromJson(jsonObj);
			//this->sendSignal(networkReply->readAll());
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
}