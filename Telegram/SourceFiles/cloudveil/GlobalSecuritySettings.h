#pragma once
#include "./response/SettingsResponse.h"
#include "chat_helpers/stickers.h"
#include "data/data_document.h"

class SettingsRequest;

class GlobalSecuritySettings: public QObject, public RPCSender
{
	Q_OBJECT

public slots:
	void requestFinished(QNetworkReply *networkReply);

signals:
	void settingsReady();

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
	PeerData *additionalItem;
	QVector<SettingsRequest::Row> additionalStickers;

	static bool loaded;
	static SettingsResponse lastResponse;
	static GlobalSecuritySettings *instance;

private slots:
	void doServerRequest();

private:
	void buildRequest(SettingsRequest &request);
	void addDialogToRequest(SettingsRequest &request, PeerData *peerData);
	void addStickerToRequest(SettingsRequest &request, Stickers::Set &set);
	void addStickerToRequest(SettingsRequest &request, const MTPDstickerSet *additionalSticker);
	void sendRequest(SettingsRequest &request);

	void gotStickersSet(const MTPmessages_StickerSet &set);
	bool failedStickersSet(const RPCError &error);
};