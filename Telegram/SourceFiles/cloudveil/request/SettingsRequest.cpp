#include "stdafx.h"
#include "SettingsRequest.h"
#include <QtCore/QJsonArray>
#include <QtCore/QUuid>

void SettingsRequest::Row::writeToJson(QJsonObject &json, bool writeIsMegagroup, bool writeIsPublic) {
    json["id"] = QString::number(id);
    json["title"] = title;
    if (writeIsMegagroup) {
        json["is_megagroup"] = isMegagroup;
    }
    if (writeIsPublic) {
        json["is_public"] = isPublic;
    }
    if (!userNames.isEmpty()) {
        auto arr = QJsonArray::fromStringList(userNames);
        json["user_names"] = arr;
    }
}

bool SettingsRequest::Row::equalsTo(Row& r) {
    return this->id == r.id &&
        this->title == r.title &&
        this->isMegagroup == r.isMegagroup &&
        this->isPublic == r.isPublic &&
        this->userNames == r.userNames;
}

void SettingsRequest::writeToJson(QJsonObject & json)
{
	json["user_id"] = (qint64)userId;
	json["user_phone"] = userPhone;
	json["user_name"] = userName;
	if (!userNames.isEmpty()) {
		auto arr = QJsonArray::fromStringList(userNames);
		json["user_names"] = arr;
	}
	json["client_os_type"] = clientOsType;
	json["client_session_id"] = clientSessionId;
	json["client_version_code"] = clientVersionCode;
	json["client_version_name"] = clientVersionName;

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

void SettingsRequest::writeArrayToJson(QJsonArray& jsonArray, QVector<Row>& objects, bool writeIsMegagroup, bool writeIsPublic)
{
	for (int i = 0; i < objects.size(); i++)
	{
		Row& row = objects[i];
		QJsonObject json;
		row.writeToJson(json, writeIsMegagroup, writeIsPublic);
		jsonArray.append(json);
	}
}

bool SettingsRequest::equalsTo(SettingsRequest& request)
{
	if (this->userId != request.userId ||
		this->userPhone != request.userPhone || 
		this->userName != request.userName ||
		this->userNames != request.userNames) {
		return false;
	}

	return arraysEqual(this->groups, request.groups) &&
		arraysEqual(this->channels, request.channels) &&
		arraysEqual(this->bots, request.bots) &&
		arraysEqual(this->stickers, request.stickers) &&
		arraysEqual(this->users, request.users);

}

bool SettingsRequest::arraysEqual(QVector<Row> array1, QVector<Row> array2) {
	if (array1.size() != array2.size()) {
		return false;
	}

	for (int i = 0; i < array1.size(); i++)
	{
		Row& row1 = array1[i];
		Row& row2 = array2[i];
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
