#pragma once
#include "cloudveil/request/SettingsRequest.h"
#include "chat_helpers/stickers.h"
#include "data/data_document.h"

class SettingsResponse
{
public:
	QMap<int32, bool> channels;
	QMap<int32, bool> bots;
	QMap<int32, bool> groups;
	QMap<uint64, bool> stickers;
	QMap<int32, bool> users;


	bool secretChat;
	int secretChatMinimumLength;

	bool disableBio;
	bool disableBioChange;
	bool disableProfilePhoto;
	bool disableProfilePhotoChange;
	bool disableStickers;
	bool disableGifs = true;
	bool manageUsers;

	void readFromJson(QJsonObject &jsonObject);

public:
	void saveToCache();
	static SettingsResponse loadFromCache();

	bool isDialogAllowed(History *history); 
	bool isDialogAllowed(PeerData *peer);
	bool isDialogSecured(PeerData *peer);
	bool isStickerSetAllowed(Stickers::Set &set);
	bool isStickerSetKnown(DocumentData *data);
	bool isStickerSetAllowed(StickerData *stickerData);
	bool isStickerSetAllowed(DocumentData *data);
	bool isStickerSetAllowed(uint64 id);
	Stickers::Pack filterStickersPack(Stickers::Pack &pack);

	SettingsResponse();
	~SettingsResponse();

private:
	template<typename T> void readAcccessObject(QJsonObject& accessObject, QString key, QMap<T, bool> &objects);
	template<typename T> void readArrayFromJson(QJsonArray &jsonArray, QMap<T, bool> &objects);

	void writeToJson(QJsonObject &json);
	template<typename T> void writeAcccessObject(QJsonObject& accessObject, QString key, QMap<T, bool> &objects);
	template<typename T> void writeArrayToJson(QJsonArray &jsonArray, QMap<T, bool> channels);
};

