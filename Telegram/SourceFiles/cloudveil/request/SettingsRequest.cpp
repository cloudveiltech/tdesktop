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

template<typename T> void SettingsRequest::writeArrayToJson(QJsonArray& jsonArray, QVector<Row<T>>& objects, bool writeIsMegagroup, bool writeIsPublic)
{
	for (int i = 0; i < objects.size(); i++)
	{
		Row<T>& row = objects[i];
		QJsonObject json;
		row.writeToJson(json, writeIsMegagroup, writeIsPublic);
		jsonArray.append(json);
	}
}

bool SettingsRequest::equalsTo(SettingsRequest& request)
{
	if (this->userId != request.userId ||
		this->userPhone != request.userPhone || 
		this->userName != request.userName) {
		return false;
	}

	return arraysEqual(this->groups, request.groups) &&
		arraysEqual(this->channels, request.channels) &&
		arraysEqual(this->bots, request.bots) &&
		arraysEqual(this->stickers, request.stickers) &&
		arraysEqual(this->users, request.users);

}

template<typename T> bool SettingsRequest::arraysEqual(QVector<Row<T>> array1, QVector<Row<T>> array2) {
	if (array1.size() != array2.size()) {
		return false;
	}

	for (int i = 0; i < array1.size(); i++)
	{
		Row<T>& row1 = array1[i];
		Row<T>& row2 = array2[i];
		if (!row1.equalsTo(row2)) {
			return false;
		}
	}
	return true;
}

SettingsRequest::SettingsRequest()
{
}


SettingsRequest::~SettingsRequest()
{
}
