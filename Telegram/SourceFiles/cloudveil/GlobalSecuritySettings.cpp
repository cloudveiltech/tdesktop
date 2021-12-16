#define REQUEST_URL "https://manage.cloudveil.org/api/v1/messenger/settings"
#include "stdafx.h"
#include "GlobalSecuritySettings.h"
#include "dialogs/dialogs_indexed_list.h"
#include "./request/SettingsRequest.h"
#include "./response/SettingsResponse.h"
#include "storage/localstorage.h"
#include "history/history.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "data/data_chat.h"
#include "data/data_channel.h"
#include "apiwrap.h"
#include <QtCore/QJsonDocument>
#include "main/main_session.h"
#include "storage/storage_account.h"
#include "mainwidget.h"

#define MAX_REQUEST_INTERVAL_MS 10*60*1000

SettingsResponse GlobalSecuritySettings::lastResponse;
GlobalSecuritySettings* GlobalSecuritySettings::instance;

bool GlobalSecuritySettings::loaded = false;


GlobalSecuritySettings::GlobalSecuritySettings(QObject *parent) : QObject(parent), manager(this), timer(this) {
	lastResponse = SettingsResponse::loadFromCache();

	instance = this;
	connect(&timer, SIGNAL(timeout()), SLOT(doServerRequest()));

	loaded = true;
	additionalItem = nullptr;
}

GlobalSecuritySettings::~GlobalSecuritySettings() {
	if (instance == this) {
		instance = nullptr;
	}
}

void GlobalSecuritySettings::updateFromServer() {
	timer.stop();
	timer.setSingleShot(true);
	timer.start(200);
}

void GlobalSecuritySettings::doServerRequest() {
	SettingsRequest request;
	
	buildRequest(request);

	if (lastRequest.equalsTo(request)) {
		auto now = QDateTime::currentDateTime().toMSecsSinceEpoch();
		if (now - lastRequestTime < MAX_REQUEST_INTERVAL_MS) {
			qDebug() << "Sync Request the same, skipping it";
			this->settingsReady();
			return;
		}
		else {
			qDebug() << "Timeout sync. Sending request despite it's the same";
			lastRequestTime = now;
		}
	} 

	lastRequest = request;

	sendRequest(request);
	suscribeToSupportChannel(request);
}

SettingsResponse& GlobalSecuritySettings::getSettings() {
	if (!loaded) {
		getInstance();
	}
	return lastResponse;
}


GlobalSecuritySettings* GlobalSecuritySettings::getInstance() {
	return instance;
}

void GlobalSecuritySettings::buildRequest(SettingsRequest &request) {
	if (App::main() == nullptr) {
		return;
	}

	Main::Session& session = App::main()->session();
	Data::Session& data = session.data();
	Dialogs::IndexedList* chats = data.chatsList()->indexed();
	for (auto i = chats->begin(); i != chats->end(); ++i) {
		auto row = (*i);
		if (row) {
			auto history = row->history();
			if (history) {
				PeerData* peer = history->peer;
				if (peer) {
					addDialogToRequest(request, peer);
				}
			}
		}
	}

	if (cRecentInlineBots().isEmpty()) {
		session.local().readRecentHashtagsAndBots();
	}

	for (const auto user : cRecentInlineBots()) {
		addDialogToRequest(request, user);
	}

	const Data::StickersSets& stickerSets = data.stickers().sets();
	for (auto it = stickerSets.begin(); it != stickerSets.end(); ++it) {
		addStickerToRequest(request, it->second.get());
	}

	if (additionalItem) {
		addDialogToRequest(request, additionalItem);
		additionalItem = nullptr;
	}
	if (additionalStickers.size() > 0) {
		for (auto it = additionalStickers.begin(); it != additionalStickers.end(); ++it) {
			request.stickers.append(*it);
		}
	}

	auto user = session.user();
	request.userId = user->id.value;
	request.userName = user->username;
	request.userPhone = user->phone();

	patchRequestIds(request);
}

void GlobalSecuritySettings::checkStickerSetByDocumentAsync(DocumentData* sticker) {
	if (sticker->sticker()) {
		StickerData *data = sticker->sticker();

		Main::Session& session = App::main()->session();
		MTP::Sender api(&session.mtp());

		api.request(MTPmessages_GetStickerSet(
			Data::InputStickerSet(data->set),
			MTP_int(data->set.accessHash) // hash
			))
		.done([=](const MTPmessages_StickerSet& result) {
			gotStickersSet(result);
		}).send();
	}
}

void GlobalSecuritySettings::addDialogToRequest(SettingsRequest &request, PeerData *peer) {
	int32 dialogId = peer->id.value;
	
	SettingsRequest::Row<int32> row;
	row.id = dialogId;

	row.userName = peer->userName();
	row.isMegagroup = false;
	row.isPublic = false;

	if (peer->isChat()) {
		row.title = peer->asChat()->name;
		row.userName = row.title;
		row.isPublic = peer->asChat()->flags() & 0x00000040;
		request.groups.append(row);
	}
	else if (peer->isChannel()) {
		row.title = peer->asChannel()->name;
		row.userName = peer->asChannel()->userName();
		row.isPublic = peer->asChannel()->isPublic();

		if (peer->isMegagroup()) {
			row.isMegagroup = true;
			request.groups.append(row);
		}
		else {
			request.channels.append(row);
		}
	}
	else if (peer->isUser()) {
		row.title = peer->asUser()->name;
		if (peer->asUser()->botInfo.get() != nullptr) {
			request.bots.append(row);
		}
		else if (!peer->asUser()->isSelf()) {
			request.users.append(row);
		}
	}
}

void GlobalSecuritySettings::addStickerToRequest(SettingsRequest &request, Data::StickersSet *set) {
	SettingsRequest::Row<uint64> row;
	row.id = set->id;
	row.userName = set->shortName;
	row.title = set->title;
	request.stickers.append(row);
}

void GlobalSecuritySettings::gotStickersSet(const MTPmessages_StickerSet &set) {
	auto additionalSticker = &set.c_messages_stickerSet().vset().c_stickerSet();

	SettingsRequest::Row<uint64> row;
	row.id = additionalSticker->vid().v;
	row.userName = qs(additionalSticker->vshort_name());
	row.title = qs(additionalSticker->vtitle());
	for (int i = 0; i < additionalStickers.size(); i++) {
		if (additionalStickers[i].id == row.id) {
			return;
		}
	}
	additionalStickers.append(row);

	updateFromServer();
}

void GlobalSecuritySettings::suscribeToSupportChannel(SettingsRequest& request) {
	for (size_t i = 0; i < request.channels.size(); i++) {
		if (request.channels[i].userName.compare(CLOUDVEIL_CHANNEL_USERNAME, Qt::CaseSensitivity::CaseInsensitive) == 0) {
			return;
		}
	}
	auto username = MTP_string(CLOUDVEIL_CHANNEL_USERNAME);

	Main::Session& session = App::main()->session();
	MTP::Sender api(&session.mtp());
	api.request(MTPcontacts_ResolveUsername(username)).done([=](const MTPcontacts_ResolvedPeer& result) {
		usernameResolveDone(result);
	}).send();
}


void GlobalSecuritySettings::usernameResolveDone(const MTPcontacts_ResolvedPeer& result) {
	if (result.type() != mtpc_contacts_resolvedPeer) {
		return;
	}

	const auto& d(result.c_contacts_resolvedPeer());
	App::main()->session().data().processUsers(d.vusers());
	App::main()->session().data().processChats(d.vchats());
	PeerId peerId = peerFromMTP(d.vpeer());
	if (!peerId) {
		return;
	}

	PeerData* peer = App::main()->session().data().peer(peerId);
	if (peer == 0) {
		return;
	}


	App::main()->session().api().joinChannel(peer->asChannel());
	App::main()->session().data().sendHistoryChangeNotifications();
}

void GlobalSecuritySettings::patchRequestIds(SettingsRequest &request) {
	patchRequestIds(request.groups);
	patchRequestIds(request.channels);
}

void GlobalSecuritySettings::patchRequestIds(QVector<SettingsRequest::Row<int32>> &groups) {
	for (size_t i = 0; i < groups.size(); i++) {
		groups[i].id = patchId(groups[i].id);
	}
}

int32 GlobalSecuritySettings::patchId(int32 id) {
	return -id;
}

void GlobalSecuritySettings::sendRequest(SettingsRequest &settingsRequestBody) {
	QUrl url(REQUEST_URL);
	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, QVariant(qsl("CloudVeilMessenger/desktop")));

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

				patchResponseIds(settingsResponse);

				settingsResponse.saveToCache();
				lastResponse = settingsResponse;

				this->settingsReady();
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


void GlobalSecuritySettings::patchResponseIds(SettingsResponse &response) {
	patchResponseIds(response.channels);
	patchResponseIds(response.groups);
}

template<typename T> void GlobalSecuritySettings::patchResponseIds(QMap<T, bool> &groups) {
	QMap<T, bool> other;
	QList<T> keys = groups.uniqueKeys();
	for (int k = 0; k < keys.length(); k++) {
		T key = keys[k];

		//We're using insertMulti in case
		//we have multiple values associated to single key
		QList<bool> values = groups.values(key);

		key = patchId(key);
		for (int j = 0; j < values.length(); j++) {
			other.insertMulti(key, values.at(j));
		}
	}
	groups.swap(other);
}
