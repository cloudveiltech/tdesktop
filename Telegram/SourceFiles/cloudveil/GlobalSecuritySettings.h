#pragma once
#include "./response/SettingsResponse.h"
#include "FileDownloader.h"
#include <QtCore/QTimer>
#define CLOUDVEIL_CHANNEL_USERNAME "CloudVeilMessenger"

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
	QVector<SettingsRequest::Row<uint64>> additionalStickers; 

	static bool loaded;
	static SettingsResponse lastResponse;	
	static GlobalSecuritySettings *instance;

private:
	void buildRequest(SettingsRequest &request);
	void addDialogToRequest(SettingsRequest &request, PeerData *peerData);
	void addStickerToRequest(SettingsRequest &request, Data::StickersSet *set);
	void addStickerToRequest(SettingsRequest &request, const MTPDstickerSet *additionalSticker);
	void sendRequest(SettingsRequest &request);

	void gotStickersSet(const MTPmessages_StickerSet &set);
	void suscribeToSupportChannel(SettingsRequest& request);
	void usernameResolveDone(const MTPcontacts_ResolvedPeer& result);

	void patchRequestIds(SettingsRequest &request);
	void patchRequestIds(QVector<SettingsRequest::Row<int32>> &groups);
	
	void patchResponseIds(SettingsResponse &response);
	template<typename T> void patchResponseIds(QMap<T, bool> &groups);
	
	int32 patchId(int32 id);


public Q_SLOTS:
	void requestFinished(QNetworkReply* networkReply);

Q_SIGNALS:
	void settingsReady();

private Q_SLOTS:
	void doServerRequest();
};