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
	void addAdditionalDataToRequest(PeerData *peer) {
		additionalItem = peer;
	}

	void updateFromServer();

	explicit GlobalSecuritySettings(QObject *parent);
	~GlobalSecuritySettings();

	static SettingsResponse& getSettings();

	static GlobalSecuritySettings* getInstance();
private:
	QNetworkAccessManager manager;
	SettingsRequest lastRequest;

	QTimer timer;
	PeerData *additionalItem;

	static bool loaded;
	static SettingsResponse lastResponse;
	static GlobalSecuritySettings *instance;

private slots:
	void doServerRequest();

private:
	void buildRequest(SettingsRequest &request);
	void addDialogToRequest(SettingsRequest &request, PeerData *peerData);
	void sendRequest(SettingsRequest &request);
};

