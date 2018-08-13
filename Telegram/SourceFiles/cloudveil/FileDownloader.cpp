#include "FileDownloader.h"

FileDownloader::FileDownloader(QObject *parent) :
	QObject(parent)
{
	connect(
		&webCtrl, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(fileDownloaded(QNetworkReply*))
	);	
}

void FileDownloader::download(QUrl imageUrl) {
	QNetworkRequest request(imageUrl);
	webCtrl.get(request);
}


void FileDownloader::fileDownloaded(QNetworkReply* networkReply) {
	qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if (httpStatusCode >= 200 && httpStatusCode < 300) // OK
	{
		downloadedData = networkReply->readAll();
		//emit a signal
		networkReply->deleteLater();
		emit downloaded();
	}	
}