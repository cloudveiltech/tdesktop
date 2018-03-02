#pragma once
#include "./response/SettingsResponse.h"

class SettingsRequest;

class GlobalSecuritySettings: public QObject
{
	Q_OBJECT

public slots:
	void requestFinished(QNetworkReply *networkReply);

signals:
	void settingsReady();

public:
	void updateFromServer();

	explicit GlobalSecuritySettings(QObject *parent);

	static SettingsResponse& getSettings();
private:
	QNetworkAccessManager manager;
	QTimer timer;

	static bool loaded;
	static SettingsResponse lastResponse;

private slots:
	void doServerRequest();

private:
	void buildRequest(SettingsRequest &request);
	void sendRequest(SettingsRequest &request);
};

