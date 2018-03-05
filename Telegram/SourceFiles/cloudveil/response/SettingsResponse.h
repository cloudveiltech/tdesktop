#pragma once
class SettingsResponse
{
public:
	QVector<int32> channels;
	QVector<int32> bots;
	QVector<int32> groups;

	bool secretChat;
	int secretChatMinimumLength;

	bool disableBio;
	bool disableBioChange;
	bool disableProfilePhoto;
	bool disableProfilePhotoChange;

	void readFromJson(QJsonObject &jsonObject);

public:
	void saveToCache();
	static SettingsResponse loadFromCache();

	bool isGroupAllowed(const MTPChat &group);
	bool isUserAllowed(const MTPUser &user);

	SettingsResponse();
	~SettingsResponse();
private:
	void readArrayFromJson(QJsonArray &jsonArray, QVector<int32> &objects);

	void writeToJson(QJsonObject &json);
	void writeArrayToJson(QJsonArray &jsonArray, QVector<int32> &objects);
};

