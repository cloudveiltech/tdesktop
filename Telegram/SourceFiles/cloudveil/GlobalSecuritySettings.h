#pragma once
#include "dialogs/dialogs_indexed_list.h"
#include "./response/SettingsResponse.h"
#include "FileDownloader.h"
#include <QtCore/QTimer>
#define CLOUDVEIL_CHANNEL_USERNAME "CloudVeilMessenger"
#define PREMIUM_BOT_ID 5314653481

class SettingsRequest;

class GlobalSecuritySettings: public QObject
{
	Q_OBJECT

public:
	void addAdditionalDataToRequest(PeerData *peer) {
		additionalItem = peer;
	}

	void checkStickerSetByDocumentAsync(DocumentData* sticker);

	void updateFromServer();

	explicit GlobalSecuritySettings(QObject *parent);

	~GlobalSecuritySettings();

	static SettingsResponse& getSettings();

	static GlobalSecuritySettings* getInstance();
private:
	QNetworkAccessManager manager; 
	QTimer timer;
	SettingsRequest lastRequest;
	qint64 lastRequestTime;
	PeerData *additionalItem;
	QVector<SettingsRequest::Row> additionalStickers; 

	static bool loaded;
	static SettingsResponse lastResponse;	
	static GlobalSecuritySettings *instance;

	QMap<qint64, QString> sessionUids;

private:
	void buildRequest(SettingsRequest &request);
	void addDialogToRequest(SettingsRequest &request, PeerData *peerData);
	void addStickerToRequest(SettingsRequest &request, Data::StickersSet *set);
	void addStickerToRequest(SettingsRequest &request, const MTPDstickerSet *additionalSticker);
	void sendRequest(SettingsRequest &request);
	void addChatListToRequest(SettingsRequest& request, Dialogs::IndexedList* chats, QStack<Dialogs::IndexedList*> &listsStack);

	void gotStickersSet(const MTPmessages_StickerSet &set);
	void suscribeToSupportChannel(SettingsRequest& request);
	void usernameResolveDone(const MTPcontacts_ResolvedPeer& result);		

	void patchRequestIds(SettingsRequest& request);
	void patchRequestIds(QVector<SettingsRequest::Row>& groups);

	void patchResponseIds(SettingsResponse& response); 
	void patchResponseIds(QMap<qint64, bool>& groups);
	QString getUserSessionId(qint64 userId);

	qint64 patchId(qint64 id);
public Q_SLOTS:
	void requestFinished(QNetworkReply* networkReply);

Q_SIGNALS:
	void settingsReady();

private Q_SLOTS:
	void doServerRequest();
};