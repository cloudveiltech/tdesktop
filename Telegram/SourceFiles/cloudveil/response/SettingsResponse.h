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
		int id;
		bool needChange;
		QString aboutUrl;

		QJsonObject toJsonObject();
		void readFromJson(QJsonObject& object);
	};

public:
	QMap<qint64, bool> channels;
	QMap<qint64, bool> bots;
	QMap<qint64, bool> groups;
	QMap<qint64, bool> stickers;
	QMap<qint64, bool> users;

	bool disableBio;
	bool disableBioChange;
	bool disableProfilePhoto;
	bool disableProfilePhotoChange;
	bool disableStickers;
	bool disableGifs = true;
	int profilePhotoLimit;
	bool disableProfileVideo;
	bool disableProfileVideoChange;
	bool disableEmojiStatus;
	bool manageUsers;
	bool disableStories;
	Organization orgranization;

public:
	void readFromJson(QJsonObject &jsonObject);

	void saveToCache();
	static SettingsResponse loadFromCache();

	bool isDialogAllowed(History *history); 
	bool isDialogAllowed(PeerData *peer);
	bool isDialogSecured(PeerData *peer);
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

