#include "stdafx.h"
#include "SettingsResponse.h"
#include "mainwidget.h"
#include "dialogs/dialogs_indexed_list.h"

void SettingsResponse::readFromJson(QJsonObject &jsonObject, SettingsRequest &request)
{
	readFromJson(jsonObject);

	addForbiddenArray(botsAllowed, request.bots, botsForbidden);
	addForbiddenArray(groupsAllowed, request.groups, groupsForbidden);
	addForbiddenArray(channelsAllowed, request.channels, channelsForbidden);
}


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


	if (jsonObject.contains("channels") && jsonObject["channels"].isArray())
	{
		QJsonArray channelsJson = jsonObject["channels"].toArray();
		readArrayFromJson(channelsJson, channelsAllowed);
	}
	if (jsonObject.contains("bots") && jsonObject["bots"].isArray())
	{
		QJsonArray botsJson = jsonObject["bots"].toArray();
		readArrayFromJson(botsJson, botsAllowed);
	}
	if (jsonObject.contains("groups") && jsonObject["groups"].isArray())
	{
		QJsonArray groupsJson = jsonObject["groups"].toArray();
		readArrayFromJson(groupsJson, groupsAllowed);
	}
	if (jsonObject.contains("channelsForbidden") && jsonObject["channelsForbidden"].isArray())
	{
		QJsonArray channelsJson = jsonObject["channelsForbidden"].toArray();
		readArrayFromJson(channelsJson, channelsForbidden);
	}
	if (jsonObject.contains("botsForbidden") && jsonObject["botsForbidden"].isArray())
	{
		QJsonArray botsJson = jsonObject["botsForbidden"].toArray();
		readArrayFromJson(botsJson, botsForbidden);
	}
	if (jsonObject.contains("groupsForbidden") && jsonObject["groupsForbidden"].isArray())
	{
		QJsonArray groupsJson = jsonObject["groupsForbidden"].toArray();
		readArrayFromJson(groupsJson, groupsForbidden);
	}
}

void SettingsResponse::readArrayFromJson(QJsonArray &jsonArray, QVector<int32> &objects)
{
	objects.clear();
	for (int i = 0; i < jsonArray.size(); i++)
	{
		objects.append(jsonArray[i].toInt());
	}
}

void SettingsResponse::addForbiddenArray(QVector<int32>& whiteListed, QVector<SettingsRequest::Row>& requested, QVector<int32>& blackListed)
{
	for (int i = 0; i < requested.size(); i++) {
		if (!whiteListed.contains(requested[i].id)) {
			blackListed.append(requested[i].id);
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
	json["disable_bio"] = disableBio;

	QJsonArray groupsArray;
	writeArrayToJson(groupsArray, groupsAllowed);
	json["groups"] = groupsArray;

	QJsonArray channelsArray;
	writeArrayToJson(channelsArray, channelsAllowed);
	json["channels"] = channelsArray;

	QJsonArray botsArray;
	writeArrayToJson(botsArray, botsAllowed);
	json["bots"] = botsArray;

	QJsonArray groupsForbiddenArray;
	writeArrayToJson(groupsForbiddenArray, groupsForbidden);
	json["groupsForbidden"] = groupsForbiddenArray;

	QJsonArray channelsForbiddenArray;
	writeArrayToJson(channelsForbiddenArray, channelsForbidden);
	json["channelsForbidden"] = channelsForbiddenArray;

	QJsonArray botsForbiddenArray;
	writeArrayToJson(botsForbiddenArray, botsForbidden);
	json["botsForbidden"] = botsForbiddenArray;
}

void SettingsResponse::writeArrayToJson(QJsonArray &jsonArray, QVector<int32> &objects)
{
	for (int i = 0; i < objects.size(); i++)
	{
		int32 row = objects[i];
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
	if (peer->isUser()) {
		return true;
	}

	bool isInlineBot = peer->isUser() && peer->asUser()->botInfo != NULL && peer->asUser()->botInfo->inlinePlaceholder != NULL && peer->asUser()->botInfo->inlinePlaceholder.length() > 0;
		
	if (!isInlineBot && !isDialogSecured(peer)) {//unknown dialogs assumed to be allowed
		return true;
	}

	if (peer->isChannel()) {
		return channelsAllowed.indexOf(peer->bareId()) >= 0;
	}
	else if (peer->isChat()) {
		return groupsAllowed.indexOf(peer->bareId()) >= 0;
	}
	else if (peer->isUser()) {
		return peer->asUser()->botInfo == NULL || botsAllowed.indexOf(peer->bareId()) >= 0;
	}
	return false;
}

bool SettingsResponse::isDialogSecured(PeerData *peer) {
	if (peer == nullptr) {
		return true;
	}
		
	if (peer->isUser()) {
		return true;
	}

	return botsForbidden.contains(peer->id) || 
		channelsForbidden.contains(peer->id) ||
		groupsForbidden.contains(peer->id) ||
		botsAllowed.contains(peer->id) ||
		channelsAllowed.contains(peer->id) || 
		groupsAllowed.contains(peer->id);
}

SettingsResponse::SettingsResponse()
{
}


SettingsResponse::~SettingsResponse()
{
}

