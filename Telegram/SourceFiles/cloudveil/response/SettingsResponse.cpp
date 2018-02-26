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
		secretChat = jsonObject["secret_chat_minimum_length"].toInt();
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

void SettingsResponse::readArrayFromJson(QJsonArray &jsonArray, QVector<long> &objects) 
{
	objects.clear();
	for (int i = 0; i < jsonArray.size(); i++)
	{
		objects.append(jsonArray[i].toInt());
	}
}


SettingsResponse::SettingsResponse()
{
}


SettingsResponse::~SettingsResponse()
{
}

