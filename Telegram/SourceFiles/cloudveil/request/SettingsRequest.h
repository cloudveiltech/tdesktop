#pragma once
class SettingsRequest
{
public:
	template<typename T> 
	struct Row {
		T id;
		QString title;
		QString userName;

		void writeToJson(QJsonObject &json) {			
			json["id"] = QString::number(id);
			json["title"] = title;
			json["user_name"] = userName;
		}
	};

	int32 userId;
	QString userPhone;
	QString userName;

	QVector<Row<int32>> groups;
	QVector<Row<int32>> channels;
	QVector<Row<int32>> bots;
	QVector<Row<uint64>> stickers;
	QVector<Row<int32>> users;

	void writeToJson(QJsonObject &json);
	
	SettingsRequest();
	~SettingsRequest();

private:
	template<typename T> void writeArrayToJson(QJsonArray &jsonArray, QVector<Row<T>> &objects);
};

