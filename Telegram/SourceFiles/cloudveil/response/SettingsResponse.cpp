#include "stdafx.h"
#include "SettingsResponse.h"

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
		readArrayFromJson(channelsJson, channels);
	}
	if (jsonObject.contains("bots") && jsonObject["bots"].isArray())
	{
		QJsonArray botsJson = jsonObject["bots"].toArray();
		readArrayFromJson(botsJson, bots);
	}
	if (jsonObject.contains("groups") && jsonObject["groups"].isArray())
	{
		QJsonArray groupsJson = jsonObject["groups"].toArray();
		readArrayFromJson(groupsJson, groups);
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
	writeArrayToJson(groupsArray, groups);
	json["groups"] = groupsArray;

	QJsonArray channelsArray;
	writeArrayToJson(channelsArray, channels);
	json["channels"] = channelsArray;

	QJsonArray botsArray;
	writeArrayToJson(botsArray, bots);
	json["bots"] = botsArray;
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


SettingsResponse::SettingsResponse()
{
}


SettingsResponse::~SettingsResponse()
{
}

