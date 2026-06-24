#pragma once
class SettingsRequest
{
public:
	struct Row {
		qint64 id;
		QString title;
		QStringList userNames;
		bool isMegagroup;
		bool isPublic;
		qint64 migratedFromTelegramId = 0;
        
		void writeToJson(QJsonObject &json, bool writeIsMegagroup = false, bool writeIsPublic=false);
		bool equalsTo(Row& r);
	};

	qint64 userId;
	QString userPhone;
	QString userName;
	QStringList userNames;
#ifdef __APPLE__
	QString clientOsType = "macOS";
#elif defined(__linux__)
	QString clientOsType = "Linux";
#else
	QString clientOsType = "Windows";
#endif
	QString clientSessionId;
	int clientVersionCode = AppVersion;
	QString clientVersionName = AppVersionStr;

	QVector<Row> groups;
	QVector<Row> channels;
	QVector<Row> bots;
	QVector<Row> stickers;
	QVector<Row> users;

	void writeToJson(QJsonObject &json);
	bool equalsTo(SettingsRequest& request);
	bool isEmpty() const {
		return groups.size() == 0 &&
			channels.size() == 0 &&
			bots.size() == 0 &&
			stickers.size() == 0 &&
			users.size() == 0;
	}

	SettingsRequest();
	~SettingsRequest();

private:
	void writeArrayToJson(QJsonArray &jsonArray, QVector<Row> &objects, bool writeIsMegagroup=false, bool writeIsPublic=false);
	bool arraysEqual(QVector<Row> array1, QVector<Row> array2);
};

