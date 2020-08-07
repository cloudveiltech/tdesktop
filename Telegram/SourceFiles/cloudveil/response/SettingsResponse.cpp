#include "stdafx.h"
#include "SettingsResponse.h"
#include "mainwidget.h"
#include "dialogs/dialogs_indexed_list.h"
#include "data/data_document.h"
#include "history/history.h"
#include "data/data_user.h"
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

void SettingsResponse::readFromJson(QJsonObject &jsonObject)
{
	if (jsonObject.contains("secret_chat") && jsonObject["secret_chat"].isBool())
	{
		secretChat = jsonObject["secret_chat"].toBool();
	}
	if (jsonObject.contains("secret_chat_minimum_length") && jsonObject["secret_chat_minimum_length"].isDouble())
	{
		secretChatMinimumLength = jsonObject["secret_chat_minimum_length"].toInt();
	}

	if (jsonObject.contains("disable_bio") && jsonObject["disable_bio"].isBool())
	{
		disableBio = jsonObject["disable_bio"].toBool();
	}
	if (jsonObject.contains("disable_bio_change") && jsonObject["disable_bio_change"].isBool())
	{
		disableBioChange = jsonObject["disable_bio_change"].toBool();
	}
	if (jsonObject.contains("disable_profile_photo") && jsonObject["disable_profile_photo"].isBool())
	{
		disableProfilePhoto = jsonObject["disable_profile_photo"].toBool();
	}
	if (jsonObject.contains("disable_profile_photo_change") && jsonObject["disable_profile_photo_change"].isBool())
	{
		disableProfilePhotoChange = jsonObject["disable_profile_photo_change"].toBool();
	}
	if (jsonObject.contains("disable_stickers") && jsonObject["disable_stickers"].isBool())
	{
		disableStickers = jsonObject["disable_stickers"].toBool();
	}
	if (jsonObject.contains("manage_users") && jsonObject["manage_users"].isBool())
	{
		manageUsers = jsonObject["manage_users"].toBool();
	}

	if (jsonObject.contains("disable_stickers_image") && jsonObject["disable_stickers_image"].isString())
	{
		bannedImageUrl = jsonObject["disable_stickers_image"].toString();
	}


	if (jsonObject.contains("access") && jsonObject["access"].isObject())
	{
		QJsonObject accessObject = jsonObject["access"].toObject();

		readAcccessObject(accessObject, "groups", groups);
		readAcccessObject(accessObject, "bots", bots);
		readAcccessObject(accessObject, "channels", channels);
		readAcccessObject(accessObject, "stickers", stickers);
		readAcccessObject(accessObject, "users", users);
	}
}

template<typename T> void SettingsResponse::readAcccessObject(QJsonObject& accessObject, QString key, QMap<T, bool> &objects) {
	if (accessObject.contains(key) && accessObject[key].isArray()) {
		QJsonArray arrayJson = accessObject[key].toArray();
		readArrayFromJson(arrayJson, objects);
	}
}

template<typename T> void SettingsResponse::readArrayFromJson(QJsonArray &jsonArray, QMap<T, bool> &objects)
{
	objects.clear();
	for (int i = 0; i < jsonArray.size(); i++)
	{
		QJsonObject row = jsonArray[i].toObject();
		if (row.keys().size() > 0) {
			QString key = row.keys().at(0);
			bool value = row[key].toBool();

			bool success = false;
			T intKey = key.toLongLong(&success);
			if (success) {
				objects.insert(intKey, value);
			}
		}
	}
}

void SettingsResponse::writeToJson(QJsonObject &json)
{
	json["secret_chat"] = secretChat;
	json["secret_chat_minimum_length"] = secretChatMinimumLength;
	json["disable_bio"] = disableBio;
	json["disable_bio_change"] = disableBioChange;
	json["disable_profile_photo"] = disableProfilePhoto;
	json["disable_profile_photo_change"] = disableProfilePhotoChange;
	json["disable_bio"] = disableBio;
	json["disable_bio_change"] = disableBioChange;
	json["disable_stickers_image"] = bannedImageUrl;

	QJsonObject accessObject;

	writeAcccessObject(accessObject, "groups", groups);
	writeAcccessObject(accessObject, "bots", bots);
	writeAcccessObject(accessObject, "channels", channels);
	writeAcccessObject(accessObject, "stickers", stickers);
	writeAcccessObject(accessObject, "users", users);

	json["access"] = accessObject;
}


template<typename T> void SettingsResponse::writeAcccessObject(QJsonObject& accessObject, QString key, QMap<T, bool> &objects)
{
	QJsonArray jsonArray;
	writeArrayToJson(jsonArray, objects);
	accessObject[key] = jsonArray;
}

template<typename T> void SettingsResponse::writeArrayToJson(QJsonArray &jsonArray, QMap<T, bool> objects)
{
	for (auto it = objects.begin(); it != objects.end(); ++it) 
	{
		QJsonObject row;
		row[QString::number(it.key())] = it.value();
		jsonArray.append(row);
	}
}

void SettingsResponse::saveToCache()
{
	QFile saveFile(QStringLiteral("save.dat"));

	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return;
	}

	QJsonObject json;
	writeToJson(json);

	QJsonDocument saveDoc(json);
	saveFile.write(saveDoc.toBinaryData());
}


SettingsResponse SettingsResponse::loadFromCache()
{
	QFile loadFile(QStringLiteral("save.dat"));

	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning("Couldn't open save file.");
		return SettingsResponse();
	}

	QByteArray saveData = loadFile.readAll();

	QJsonDocument loadDoc(QJsonDocument::fromBinaryData(saveData));

	SettingsResponse result;
	QJsonObject json = loadDoc.object();
	result.readFromJson(json);

	if (QFile::exists(QStringLiteral("banned.dat"))) {
		App::main()->getBannedImage().load(QStringLiteral("banned.dat"));
	}

	return result;
}

bool SettingsResponse::isDialogAllowed(History *history) {
	if (history == nullptr) {
		return true;
	}
	return isDialogAllowed(history->peer);
}

bool SettingsResponse::isDialogAllowed(PeerData *peer) {
	if (peer == nullptr) {
		return true;
	}

	bool isInlineBot = peer->isUser() && peer->asUser()->botInfo != NULL && peer->asUser()->botInfo->inlinePlaceholder != NULL && peer->asUser()->botInfo->inlinePlaceholder.length() > 0;
		
	if (!isInlineBot && !isDialogSecured(peer)) {//unknown dialogs assumed to be allowed
		return true;
	}
	if (peer->isChat() || peer->isMegagroup()) {
		return groups.contains(peer->bareId()) && groups[peer->bareId()];
	}
	else if (peer->isChannel()) {
		return channels.contains(peer->bareId()) && channels[peer->bareId()];
	}
	else if (peer->isUser()) {
		if (peer->asUser()->botInfo == NULL) {
			if (manageUsers) {
				if (peer->asUser()->isSelf()) {
					return true;
				}
				return users.contains(peer->bareId()) && users[peer->bareId()];
			}
			else {
				return true;
			}
		} else {
			if (peer->userName() == "cloudveilbot") {
				return true;
			}
			return bots.contains(peer->bareId()) && bots[peer->bareId()];
		}
	}
	return false;
}

bool SettingsResponse::isDialogSecured(PeerData *peer) {
	if (peer == nullptr) {
		return true;
	}
		
	if (peer->isUser() && !manageUsers) {
		return true;
	}
	if (peer->isUser() && peer->asUser()->isSelf()) {
		return true;
	}
	return bots.contains(peer->id) ||
		channels.contains(peer->id) ||
		groups.contains(peer->id) ||
		users.contains(peer->id);
}

bool SettingsResponse::isStickerSetAllowed(Stickers::Set &set) {
	return isStickerSetAllowed(set.id);
}

bool SettingsResponse::isStickerSetAllowed(DocumentData *data) {
	StickerData *stickerData = data->sticker();

	return isStickerSetAllowed(stickerData);
}


bool SettingsResponse::isStickerSetAllowed(StickerData *stickerData) {
	if (stickerData) {
		if (stickerData->set.type() == mtpc_inputStickerSetID) {			
			uint64 id = stickerData->set.c_inputStickerSetID().vid().v;
			return isStickerSetAllowed(id);
		}
		else {
			return false;
		}
		
	}
	return !disableStickers;
}

bool SettingsResponse::isStickerSetKnown(DocumentData *data) {
	StickerData *stickerData = data->sticker();
	if (stickerData) {
		if (stickerData->set.type() == mtpc_inputStickerSetID) {
			uint64 id = stickerData->set.c_inputStickerSetID().vid().v;
			return stickers.contains(id);
		}
		else {//id unknown - forbid it
			return false;
		}
	}
	return !disableStickers;
}

bool SettingsResponse::isStickerSetAllowed(uint64 id) {
	if (id == 1258816259751983) {
		return true;
	}
	return !disableStickers && stickers.contains(id) && stickers[id];
}

Stickers::Pack SettingsResponse::filterStickersPack(Stickers::Pack &pack) {
	Stickers::Pack newPack;
	for (auto it = pack.begin(); it != pack.end(); ++it) {
		if (isStickerSetAllowed(*it)) {
			newPack.append(*it);
		}
	}
	return newPack;
}

void SettingsResponse::saveBannedImage() 
{
	App::main()->getBannedImage().save(QStringLiteral("banned.dat"));
}

SettingsResponse::SettingsResponse()
{
}


SettingsResponse::~SettingsResponse()
{
}

