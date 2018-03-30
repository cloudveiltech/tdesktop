#pragma once
#include "cloudveil/request/SettingsRequest.h"

class SettingsResponse
{
public:
	QVector<int32> channelsAllowed;
	QVector<int32> botsAllowed;
	QVector<int32> groupsAllowed;

	QVector<int32> channelsForbidden;
	QVector<int32> botsForbidden;
	QVector<int32> groupsForbidden;

	bool secretChat;
	int secretChatMinimumLength;

	bool disableBio;
	bool disableBioChange;
	bool disableProfilePhoto;
	bool disableProfilePhotoChange;

	void readFromJson(QJsonObject &jsonObject, SettingsRequest &request);
	void readFromJson(QJsonObject &jsonObject);

public:
	void saveToCache();
	static SettingsResponse loadFromCache();

	bool isDialogAllowed(History *history); 
	bool isDialogAllowed(PeerData *peer);
	bool isDialogSecured(PeerData *peer);

	SettingsResponse();
	~SettingsResponse();
private:
	void readArrayFromJson(QJsonArray &jsonArray, QVector<int32> &objects);
	void addForbiddenArray(QVector<int32>& whiteListed, QVector<SettingsRequest::Row>& requested, QVector<int32> &blackListed);

	void writeToJson(QJsonObject &json);
	void writeArrayToJson(QJsonArray &jsonArray, QVector<int32> &objects);
};

