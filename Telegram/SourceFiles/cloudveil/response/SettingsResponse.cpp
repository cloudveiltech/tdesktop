#include "stdafx.h"
#include <QtCore/QJsonDocument>
#include "SettingsResponse.h"
#include "settings.h"
#include "mainwidget.h"
#include "dialogs/dialogs_indexed_list.h"
#include "data/data_document.h"
#include "history/history.h"
#include "data/data_user.h"
#include "data/data_channel.h"
#include <QtCore/QJsonArray>
#include <cloudveil/DialogHelper.h>

#define EMOJI_STICKERSET_ID 1258816259751983


QJsonObject SettingsResponse::Organization::toJsonObject() {
	QJsonObject organizationObject;
	organizationObject["id"] = id;
	organizationObject["name"] = name;
	organizationObject["need_change"] = needChange;
	organizationObject["about_url"] = aboutUrl;
	organizationObject["policy_url"] = policyUrl;
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
	if (object.contains("policy_url")) {
		policyUrl = object["policy_url"].toString();
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

	if (jsonObject.contains("disable_stories") && jsonObject["disable_stories"].isBool())
	{
		disableStories = jsonObject["disable_stories"].toBool();
	}
	if (jsonObject.contains("disable_stars") && jsonObject["disable_stars"].isBool())
	{
		disableStars = jsonObject["disable_stars"].toBool();
	}
	if (jsonObject.contains("disable_mini_apps") && jsonObject["disable_mini_apps"].isBool())
	{
		disableMiniApps = jsonObject["disable_mini_apps"].toBool();
	}
	if (jsonObject.contains("disable_ai_compose") && jsonObject["disable_ai_compose"].isBool())
	{
		disableAiCompose = jsonObject["disable_ai_compose"].toBool();
	}
	if (jsonObject.contains("disable_music_status") && jsonObject["disable_music_status"].isBool())
	{
		disableMusicStatus = jsonObject["disable_music_status"].toBool();
	}
	if (jsonObject.contains("input_toggle_voice_video") && jsonObject["input_toggle_voice_video"].isBool())
	{
		inputToggleVoiceVideo = jsonObject["input_toggle_voice_video"].toBool();
	}
	if (jsonObject.contains("remove_account_url") && jsonObject["remove_account_url"].isString())
	{
		removeAccountUrl = jsonObject["remove_account_url"].toString();
	}
	if (jsonObject.contains("deprecation") && jsonObject["deprecation"].isObject())
	{
		const auto object = jsonObject["deprecation"].toObject();
		deprecation.deprecated = object["deprecated"].toBool(false);
		deprecation.message = object["message"].toString();
		deprecation.reminder = object["reminder"].toInt(0);
	}
	if (jsonObject.contains("nonblockable_bots") && jsonObject["nonblockable_bots"].isArray())
	{
		nonblockableBots.clear();
		for (const auto &value : jsonObject["nonblockable_bots"].toArray()) {
			if (value.isDouble() || value.isString()) {
				nonblockableBots.append(value.toVariant().toLongLong());
			}
		}
	}
	if (jsonObject.contains("disable_emoji_status") && jsonObject["disable_emoji_status"].isBool())
	{
		disableEmojiStatus = jsonObject["disable_emoji_status"].toBool();
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
	json["disable_stories"] = disableStories;
	json["disable_emoji_status"] = disableEmojiStatus;
	json["disable_stars"] = disableStars;
	json["disable_mini_apps"] = disableMiniApps;
	json["disable_ai_compose"] = disableAiCompose;
	json["disable_music_status"] = disableMusicStatus;
	json["input_toggle_voice_video"] = inputToggleVoiceVideo;
	json["remove_account_url"] = removeAccountUrl;
	{
		QJsonObject object;
		object["deprecated"] = deprecation.deprecated;
		object["message"] = deprecation.message;
		object["reminder"] = deprecation.reminder;
		json["deprecation"] = object;
	}
	{
		QJsonArray bots;
		for (const auto id : nonblockableBots) {
			bots.append(id);
		}
		json["nonblockable_bots"] = bots;
	}

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

// Rules are cached per Telegram account, in the app's data folder (the old
// "save.json" was relative to the process working directory).
static QString CachePath(qint64 userId) {
	return cWorkingDir() + u"tdata/cvm_settings_%1.json"_q.arg(userId);
}

void SettingsResponse::saveToCache(qint64 userId)
{
	if (!userId) {
		return;
	}
	QFile saveFile(CachePath(userId));

	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return;
	}

	QJsonObject json;
	writeToJson(json);

	QJsonDocument saveDoc(json);
	saveFile.write(saveDoc.toJson());
}

SettingsResponse SettingsResponse::loadFromCache(qint64 userId)
{
	if (!userId) {
		return SettingsResponse();
	}
	QFile loadFile(CachePath(userId));

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
	if (peer->isChannel() && peer->asChannel()->isMonoforum()) {
		// Channel direct messages follow the parent channel;
		// blocked until the parent channel is known.
		const auto parent = DialogHelper::policyPeer(peer);
		return parent && isDialogAllowed(parent);
	}

	bool isInlineBot = peer->isUser() && peer->asUser()->botInfo != NULL && !peer->asUser()->botInfo->inlinePlaceholder.isEmpty() && peer->asUser()->botInfo->inlinePlaceholder.length() > 0;
		
	if (!isInlineBot && !isDialogSecured(peer)) {//unknown dialogs assumed to be allowed
		return true;
	}

	const auto dialogId = DialogHelper::getDialogId(peer);
	if (peer->isChat() || peer->isMegagroup()) {
		return groups.contains(dialogId) && groups[dialogId];
	}
	else if (peer->isChannel()) {
		return channels.contains(dialogId) && channels[dialogId];
	}
	else if (peer->isUser()) {
		if (peer->asUser()->botInfo == NULL) {
			if (peer->asUser()->isSelf()) {
				return true;
			}
			// CloudVeil: an explicit allow/block for a user always applies,
			// even without manage_users (matches Android isUserAllowed).
			if (users.contains(dialogId)) {
				return users[dialogId];
			}
			// Unknown users: blocked when users are managed. (With
			// manage_users, unknown users don't reach here: they are
			// "not secured" and go through the pending server check.)
			return !manageUsers;
		} else {
            if (peer->username() == "cloudveilbot") {
				return true;
			}
			return bots.contains(dialogId) && bots[dialogId];
		}
	}
	return false;
}

bool SettingsResponse::isBotNonblockable(PeerData *peer) {
	if (!peer || !peer->isUser() || !peer->asUser()->isBot()) {
		return false;
	}
	return nonblockableBots.contains(qint64(DialogHelper::getDialogId(peer)));
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
	const auto dialogId = DialogHelper::getDialogId(peer);
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

