#include "stdafx.h"
#include "SettingsRequest.h"
#include <QtCore/QJsonArray>


void SettingsRequest::writeToJson(QJsonObject & json)
{
	json["user_id"] = userId;
	json["user_phone"] = userPhone;
	json["user_name"] = userName;

	QJsonArray groupsArray;
	writeArrayToJson(groupsArray, groups, true, true);	
	json["groups"] = groupsArray;

	QJsonArray channelsArray;
	writeArrayToJson(channelsArray, channels, false, true);
	json["channels"] = channelsArray;

	QJsonArray botsArray;
	writeArrayToJson(botsArray, bots);
	json["bots"] = botsArray;

	QJsonArray stickersArray;
	writeArrayToJson(stickersArray, stickers);
	json["stickers"] = stickersArray;

	QJsonArray usersArray;
	writeArrayToJson(usersArray, users);
	json["users"] = usersArray;
}

template<typename T> void SettingsRequest::writeArrayToJson(QJsonArray &jsonArray, QVector<Row<T>> &objects, bool writeIsMegagroup, bool writeIsPublic)
{
	for (int i = 0; i < objects.size(); i++)
	{
		Row<T>& row = objects[i];
		QJsonObject json;
		row.writeToJson(json, writeIsMegagroup, writeIsPublic);
		jsonArray.append(json);
	}
}


SettingsRequest::SettingsRequest()
{
}


SettingsRequest::~SettingsRequest()
{
}
