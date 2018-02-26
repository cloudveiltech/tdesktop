#pragma once
class SettingsResponse
{
public:
	QVector<long> channels;
	QVector<long> bots;
	QVector<long> groups;

	bool secretChat;
	int secretChatMinimumLength;

	bool disableBio;
	bool disableBioChange;
	bool disableProfilePhoto;
	bool disableProfilePhotoChange;

	void readFromJson(QJsonObject &jsonObject);

	SettingsResponse();
	~SettingsResponse();
private:
	void readArrayFromJson(QJsonArray &jsonArray, QVector<long> &objects);
};

