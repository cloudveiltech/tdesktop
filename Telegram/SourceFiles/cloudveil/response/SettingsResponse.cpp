#include "stdafx.h"
#include <QtCore/QJsonDocument>
#include "SettingsResponse.h"
#include "mainwidget.h"
#include "dialogs/dialogs_indexed_list.h"
#include "data/data_document.h"
#include "history/history.h"
#include "data/data_user.h"
#include <QtCore/QJsonArray>

#define EMOJI_STICKERSET_ID 1258816259751983


QJsonObject SettingsResponse::Organization::toJsonObject() {
	QJsonObject organizationObject;
	organizationObject["id"] = id;
	organizationObject["name"] = name;
	organizationObject["need_change"] = needChange;
	return organizationObject;
}

void SettingsResponse::Organization::readFromJson(QJsonObject& object) {
	if (object.contains("id")) {
		id = object["id"].toInt();
	}
	if (object.contains("name")) {
		name = object["name"].toString();
	}
	if (object.contains("need_change") && object["need_change"].isBool()) {
		needChange = object["need_change"].toBool();
	}
	if (object.contains("about_url")) {
		aboutUrl = object["about_url"].toString();
	}
}

void SettingsResponse::readFromJson(QJsonObject &jsonObject)
{
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

	if (jsonObject.contains("profile_photo_limit"))
	{
		profilePhotoLimit = jsonObject["profile_photo_limit"].toString().toInt();
	}

	if (jsonObject.contains("disable_profile_video") && jsonObject["disable_profile_video"].isBool())
	{
		disableProfileVideo = jsonObject["disable_profile_video"].toBool();
	}

	if (jsonObject.contains("disable_profile_video_change") && jsonObject["disable_profile_video_change"].isBool())
	{
		disableProfileVideoChange = jsonObject["disable_profile_video_change"].toBool();
	}

	if (jsonObject.contains("organization") && jsonObject["organization"].isObject())
	{
		auto organizationJson = jsonObject["organization"].toObject();
		orgranization.readFromJson(organizationJson);
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
	json["disable_bio"] = disableBio;
	json["disable_bio_change"] = disableBioChange;
	json["disable_profile_photo"] = disableProfilePhoto;
	json["disable_profile_photo_change"] = disableProfilePhotoChange;
	json["disable_bio"] = disableBio;
	json["disable_bio_change"] = disableBioChange;
	json["profile_photo_limit"] = profilePhotoLimit;
	json["disable_profile_video"] = disableProfileVideo;
	json["disable_profile_video_change"] = disableProfileVideoChange;

	QJsonObject accessObject;

	writeAcccessObject(accessObject, "groups", groups);
	writeAcccessObject(accessObject, "bots", bots);
	writeAcccessObject(accessObject, "channels", channels);
	writeAcccessObject(accessObject, "stickers", stickers);
	writeAcccessObject(accessObject, "users", users);

	json["access"] = accessObject;

	json["organization"] = orgranization.toJsonObject();
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
	QFile saveFile(QStringLiteral("save.json"));

	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return;
	}

	QJsonObject json;
	writeToJson(json);

	QJsonDocument saveDoc(json);
	saveFile.write(saveDoc.toJson());
}

SettingsResponse SettingsResponse::loadFromCache()
{
	QFile loadFile(QStringLiteral("save.json"));

	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning("Couldn't open save file.");
		return SettingsResponse();
	}

	QByteArray saveData = loadFile.readAll();

	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

	SettingsResponse result;
	QJsonObject json = loadDoc.object();
	result.readFromJson(json);

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

	bool isInlineBot = peer->isUser() && peer->asUser()->botInfo != NULL && !peer->asUser()->botInfo->inlinePlaceholder.isEmpty() && peer->asUser()->botInfo->inlinePlaceholder.length() > 0;
		
	if (!isInlineBot && !isDialogSecured(peer)) {//unknown dialogs assumed to be allowed
		return true;
	}

	const auto dialogId = DeserializePeerId(peer->id.value).value;
	if (peer->isChat() || peer->isMegagroup()) {
		return groups.contains(dialogId) && groups[dialogId];
	}
	else if (peer->isChannel()) {
		return channels.contains(dialogId) && channels[dialogId];
	}
	else if (peer->isUser()) {
		if (peer->asUser()->botInfo == NULL) {
			if (manageUsers) {
				if (peer->asUser()->isSelf()) {
					return true;
				}
				return users.contains(dialogId) && users[dialogId];
			}
			else {
				return true;
			}
		} else {
			if (peer->userName() == "cloudveilbot") {
				return true;
			}
			return bots.contains(dialogId) && bots[dialogId];
		}
	}
	return false;
}

bool SettingsResponse::isDialogSecured(PeerData *peer) {
	if (peer == nullptr) {
		return true;
	}
	
	if (peer->isUser() && !manageUsers) {
		if (peer->asUser()->botInfo == NULL) {
			return true;
		}
	}
	if (peer->isUser() && peer->asUser()->isSelf()) {
		return true;
	}
	const auto dialogId = DeserializePeerId(peer->id.value).value;
	return bots.contains(dialogId) ||
		channels.contains(dialogId) ||
		groups.contains(dialogId) ||
		users.contains(dialogId);
}

bool SettingsResponse::isStickerSetAllowed(Data::StickersSet &set) {
	return isStickerSetAllowed(set.id);
}

bool SettingsResponse::isStickerSetAllowed(Data::StickersSet* set) {
	return isStickerSetAllowed(set->id);
}

bool SettingsResponse::isStickerSetAllowed(DocumentData *data) {
	StickerData *stickerData = data->sticker();

	return isStickerSetAllowed(stickerData);
}


bool SettingsResponse::isStickerSetAllowed(StickerData *stickerData) {
	if (stickerData) {
		uint64 id = stickerData->set.id;
		return isStickerSetAllowed(id);
		
	}
	return !disableStickers;
}

bool SettingsResponse::isStickerSetKnown(DocumentData *data) {
	StickerData *stickerData = data->sticker();
	if (stickerData) {
		uint64 id = stickerData->set.id;
		return stickers.contains(id);
	}
	return !disableStickers;
}

bool SettingsResponse::isStickerSetAllowed(uint64 id) {
	if (id == EMOJI_STICKERSET_ID) {
		return true;
	}
	return !disableStickers && stickers.contains(id) && stickers[id];
}

SettingsResponse::SettingsResponse()
{
}


SettingsResponse::~SettingsResponse()
{
}

