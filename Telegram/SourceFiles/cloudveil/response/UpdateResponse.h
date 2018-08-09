#pragma once
class UpdateResponse
{
public:
	QString url;
	QString message;

	void readFromJson(QJsonObject &jsonObject);


	UpdateResponse();
	~UpdateResponse();
};

