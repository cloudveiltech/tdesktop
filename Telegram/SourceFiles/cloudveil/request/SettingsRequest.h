#pragma once
class SettingsRequest
{
public:
	struct Row {
		qint64 id;
		QString title;
		QString userName;
		bool isMegagroup;
		bool isPublic;

		void writeToJson(QJsonObject &json, bool writeIsMegagroup = false, bool writeIsPublic=false) {			
			json["id"] = QString::number(id);
			json["title"] = title;
			json["user_name"] = userName;
			if (writeIsMegagroup) {
				json["is_megagroup"] = isMegagroup;
			}
			if (writeIsPublic) {
				json["is_public"] = isPublic;
			}
		}

		bool equalsTo(Row& r) {
			return this->id == r.id && 
				this->title == r.title &&
				this->userName == r.userName && 
				this->isMegagroup == r.isMegagroup && 
				this->isPublic == r.isPublic;
		}
	};

	qint64 userId;
	QString userPhone;
	QString userName;
#ifdef __APPLE__
	QString clientOsType = "macOS";
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

