#pragma once
class SettingsRequest
{
public:
	struct Row {
		int32 id;
		QString title;
		QString userName;

		void writeToJson(QJsonObject &json);
	};

	int32 userId;
	QString userPhone;
	QString userName;

	QVector<Row> groups;
	QVector<Row> channels;
	QVector<Row> bots;

	void writeToJson(QJsonObject &json);

	SettingsRequest();
	~SettingsRequest();

private:
	void writeArrayToJson(QJsonArray &jsonArray, QVector<Row> &objects);
};

