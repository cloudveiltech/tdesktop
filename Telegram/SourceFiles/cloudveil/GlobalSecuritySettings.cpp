#define REQUEST_URL "https://messenger.cloudveil.org/api/v1/messenger/settings"
#include "stdafx.h"
#include "GlobalSecuritySettings.h"
#include "data/data_folder.h"
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
#include <QtCore/QStack>
#include "main/main_session.h"
#include "storage/storage_account.h"
#include "core/application.h"
#include "./DialogHelper.h"

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
	timer.start(100);
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

	if (!request.isEmpty()) {
		sendRequest(request);
	}
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
	if (!Core::IsAppLaunched()) {
		return;
	}

	Main::Session* s = Core::App().maybePrimarySession();
	if (s == nullptr) {
		return;
	}
	Main::Session& session = *s;

	Data::Session& data = session.data();
	Dialogs::IndexedList* chats = data.chatsList()->indexed();
	QStack<Dialogs::IndexedList*> lists;
	lists.push(chats);
	while (!lists.isEmpty()) {
		chats = lists.pop();
		addChatListToRequest(request, chats, lists);
	}

	if (cRecentInlineBots().isEmpty()) {
		session.local().readRecentHashtagsAndBots();
	}

	for (const auto user : cRecentInlineBots()) {
		addDialogToRequest(request, user);
	}

	const Data::StickersSets& stickerSets = data.stickers().setsRef();
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
	request.userName = user->username();
	request.userPhone = user->phone();

	std::vector<QString> userNames = user->usernames();
	request.userNames = QStringList(userNames.begin(), userNames.end());

	if (!sessionUids.contains(request.userId)) {
		sessionUids[request.userId] = getUserSessionId(request.userId);
	}
	request.clientSessionId = sessionUids[request.userId];

	patchRequestIds(request);
}

void GlobalSecuritySettings::addChatListToRequest(SettingsRequest& request, Dialogs::IndexedList* chats, QStack<Dialogs::IndexedList*> &listsStack) {
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
			else {
				auto folder = row->folder();
				if (folder) {
					listsStack.push(folder->chatsList()->indexed());
				}
			}
		}
	}
}

void GlobalSecuritySettings::checkStickerSetByDocumentAsync(DocumentData* sticker) {
	if (sticker->sticker()) {
		StickerData *data = sticker->sticker();

		Main::Session* session = Core::App().maybePrimarySession();
		if (session == nullptr) {
			return;
		}

		MTP::Sender api(&session->mtp());

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
	SettingsRequest::Row row;
	row.isMegagroup = false;
	row.isPublic = false;
    
	// peer->usernames() will internally handle the underlying user/channel/chat object.
	std::vector<QString> userNames = peer->usernames();
	row.userNames = QStringList(userNames.begin(), userNames.end());

	row.id = DialogHelper::getDialogId(peer);
	if (peer->isChat()) {
		row.title = peer->asChat()->name();
		row.isPublic = peer->isMegagroup() && peer->asChannel()->isPublic(); //copied from delete_messages_box.cpp
		request.groups.append(row);
	}
	else if (peer->isChannel()) {
		row.title = peer->asChannel()->name();
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
		row.title = peer->asUser()->name();
		if (peer->asUser()->botInfo.get() != nullptr) {
			request.bots.append(row);
		}
		else if (!peer->asUser()->isSelf()) {
			request.users.append(row);
		}
	}
}

void GlobalSecuritySettings::addStickerToRequest(SettingsRequest &request, Data::StickersSet *set) {
	SettingsRequest::Row row;
	row.id = set->id;
	auto shortName = set->shortName;
	row.userNames = QStringList{shortName};
	row.title = set->title;
	request.stickers.append(row);
}

void GlobalSecuritySettings::gotStickersSet(const MTPmessages_StickerSet &set) {
	auto additionalSticker = &set.c_messages_stickerSet().vset().c_stickerSet();

	SettingsRequest::Row row;
	row.id = additionalSticker->vid().v;
	auto shortName = qs(additionalSticker->vshort_name());
	row.userNames = QStringList{shortName};
	row.title = qs(additionalSticker->vtitle());
	for (int i = 0; i < additionalStickers.size(); i++) {
		if (additionalStickers[i].id == row.id) {
			return;
		}
	}
	additionalStickers.append(row);

	updateFromServer();
}

/*
* Deprecated
* Disable forcing to join CloudVeil Messenger Announcements channel
void GlobalSecuritySettings::suscribeToSupportChannel(SettingsRequest& request) {
	for (size_t i = 0; i < request.channels.size(); i++) {
		if (request.channels[i].userName.compare(CLOUDVEIL_CHANNEL_USERNAME, Qt::CaseSensitivity::CaseInsensitive) == 0) {
			return;
		}
	}
	auto username = MTP_string(CLOUDVEIL_CHANNEL_USERNAME);
	Main::Session* session = Core::App().maybePrimarySession();
	if (session == nullptr) {
		return;
	}
	MTP::Sender api(&session->mtp());
	api.request(MTPcontacts_ResolveUsername(MTP_flags(0),
                                            username,
                                            MTP_string())).done([=](const MTPcontacts_ResolvedPeer& result) {
		usernameResolveDone(result);
	}).send();
}
 */

void GlobalSecuritySettings::usernameResolveDone(const MTPcontacts_ResolvedPeer& result) {
	if (result.type() != mtpc_contacts_resolvedPeer) {
		return;
	}
	Main::Session* session = Core::App().maybePrimarySession();
	if (session == nullptr) {
		return;
	}

	const auto& d(result.c_contacts_resolvedPeer());
	session->data().processUsers(d.vusers());
	session->data().processChats(d.vchats());
	PeerId peerId = peerFromMTP(d.vpeer());
	if (!peerId) {
		return;
	}

	PeerData* peer = session->data().peer(peerId);
	if (peer == 0) {
		return;
	}


	session->api().joinChannel(peer->asChannel());
	session->data().sendHistoryChangeNotifications();
}


void GlobalSecuritySettings::sendRequest(SettingsRequest &settingsRequestBody) {
	QUrl url(REQUEST_URL);
	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, QVariant(qsl("CloudVeilMessenger/desktop")));

	QSslConfiguration sslConfiguration = request.sslConfiguration();
	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(sslConfiguration);

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

void GlobalSecuritySettings::patchRequestIds(SettingsRequest& request) {
	patchRequestIds(request.groups);
	patchRequestIds(request.channels);
}

void GlobalSecuritySettings::patchRequestIds(QVector<SettingsRequest::Row>& groups) {
	for (size_t i = 0; i < groups.size(); i++) {
		groups[i].id = patchId(groups[i].id);
	}
}

qint64 GlobalSecuritySettings::patchId(qint64 id) {
	return -id;
}

void GlobalSecuritySettings::patchResponseIds(SettingsResponse& response) {
	patchResponseIds(response.channels);
	patchResponseIds(response.groups);
}

void GlobalSecuritySettings::patchResponseIds(QMap<qint64, bool>& groups) {
	QMap<qint64, bool> other;
	QList<qint64> keys = groups.keys();
	for (int k = 0; k < keys.length(); k++) {
		qint64 key = keys[k];
		bool value = groups.value(key);

		key = patchId(key);
		other.insert(key, value);
	}
	groups.swap(other);
}

QString GlobalSecuritySettings::getUserSessionId(qint64 userId)
{
	QFile file(QStringLiteral("session.json"));

	QString key = QString("session_id__%1").arg(userId);
	if (!file.open(QIODevice::ReadWrite)) {
		return "";
	}	

	QByteArray data = file.readAll();
	QJsonDocument doc(QJsonDocument::fromJson(data));
	QJsonObject jsonObject;
	if (!doc.isEmpty()) {
		jsonObject = doc.object();
	}

	if (!jsonObject.contains(key)) {
		jsonObject[key] = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
		if (file.isWritable()) {		
			file.write(QJsonDocument(jsonObject).toJson());
		}
	}

	file.close();

	return jsonObject[key].toString();
}
