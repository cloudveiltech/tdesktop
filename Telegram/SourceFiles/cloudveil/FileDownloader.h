#pragma once

#include <QObject>
#include <QByteArray>

class FileDownloader : public QObject
{
	Q_OBJECT
public:
	explicit FileDownloader(QObject *parent = 0);
	void download(QUrl url);

	QByteArray data() const {
		return downloadedData;
	}

signals:
	void downloaded();

private slots:
	void fileDownloaded(QNetworkReply* pReply);

private:
	QNetworkAccessManager webCtrl;
	QByteArray downloadedData;
};
