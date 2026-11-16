#pragma once
#include "cloudveil/request/SettingsRequest.h"
#include "data/stickers/data_stickers.h"
#include "data/data_document.h"

class History;

class SettingsResponse
{
public: 
	struct Organization {
		QString name;
		int id = 0;
		bool needChange = false;
		QString aboutUrl;
		QString policyUrl;

		QJsonObject toJsonObject();
		void readFromJson(QJsonObject& object);
	};

public:
	QMap<qint64, bool> channels;
	QMap<qint64, bool> bots;
	QMap<qint64, bool> groups;
	QMap<qint64, bool> stickers;
	QMap<qint64, bool> users;

	// Defaults apply until the first server response for an account and
	// match Android's CloudVeilSecuritySettings DEFAULT_* values.
	bool disableBio = true;
	bool disableBioChange = true;
	bool disableProfilePhoto = true;
	bool disableProfilePhotoChange = true;
	bool disableStickers = false;
	bool disableGifs = true;
	int profilePhotoLimit = 1;
	bool disableProfileVideo = false;
	bool disableProfileVideoChange = false;
	bool disableEmojiStatus = true;
	bool manageUsers = false;
	bool disableStories = true;
	bool disableStars = true;
	bool disableMiniApps = true;
	bool disableAiCompose = true;
	// Bots the user may never block (Android DEFAULT_NONBLOCKABLE_BOT_ID).
	QList<qint64> nonblockableBots = { 689684671 };
	Organization orgranization;

public:
	void readFromJson(QJsonObject &jsonObject);

	void saveToCache(qint64 userId);
	static SettingsResponse loadFromCache(qint64 userId);

	bool isDialogAllowed(History *history); 
	bool isDialogAllowed(PeerData *peer);
	bool isDialogSecured(PeerData *peer);
	bool isBotNonblockable(PeerData *peer);
	bool isStickerSetAllowed(Data::StickersSet &set);
	bool isStickerSetAllowed(Data::StickersSet *set);
	bool isStickerSetKnown(DocumentData *data);
	bool isStickerSetAllowed(StickerData *stickerData);
	bool isStickerSetAllowed(DocumentData *data);
	bool isStickerSetAllowed(uint64 id);

	SettingsResponse();
	~SettingsResponse();

private:
	template<typename T> void readAcccessObject(QJsonObject& accessObject, QString key, QMap<T, bool> &objects);
	template<typename T> void readArrayFromJson(QJsonArray &jsonArray, QMap<T, bool> &objects);

	void writeToJson(QJsonObject &json);
	template<typename T> void writeAcccessObject(QJsonObject& accessObject, QString key, QMap<T, bool> &objects);
	template<typename T> void writeArrayToJson(QJsonArray &jsonArray, QMap<T, bool> channels);
};

