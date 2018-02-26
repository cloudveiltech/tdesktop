#pragma once
class SettingsRequest;

class SettingsCommand: public QObject
{
	Q_OBJECT

public slots:
	void requestFinished(QNetworkReply *networkReply);

public:
	void run();

	explicit SettingsCommand(QObject *parent);

private:
	QNetworkAccessManager manager;

private:
	void buildRequest(SettingsRequest &request);
	void sendRequest(SettingsRequest &request);
};

