#include "stdafx.h"
#include "SettingsRequest.h"


void SettingsRequest::writeToJson(QJsonObject & json)
{
	json["user_id"] = userId;
	json["user_phone"] = userPhone;
	json["user_name"] = userName;

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

void SettingsRequest::writeArrayToJson(QJsonArray &jsonArray, QVector<Row> &objects) 
{
	for (int i = 0; i < objects.size(); i++)
	{
		Row& row = objects[i];
		QJsonObject json;
		row.writeToJson(json);
		jsonArray.append(json);
	}
}

void SettingsRequest::Row::writeToJson(QJsonObject & json)
{
	json["id"] = id;
	json["title"] = title;
	json["user_name"] = userName;
}

SettingsRequest::SettingsRequest()
{
}


SettingsRequest::~SettingsRequest()
{
}
