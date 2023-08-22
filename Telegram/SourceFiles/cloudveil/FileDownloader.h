#pragma once

#include <QObject>
#include <QByteArray>
#include <QtNetwork/QNetworkReply>

class FileDownloader : public QObject
{
	Q_OBJECT
public:
	explicit FileDownloader(QObject *parent = 0);
	void download(QUrl url);

	QByteArray data() const {
		return downloadedData;
	}

Q_SIGNALS:
	void downloaded();

private Q_SLOTS:
	void fileDownloaded(QNetworkReply* pReply);

private:
	QNetworkAccessManager webCtrl;
	QByteArray downloadedData;
};
