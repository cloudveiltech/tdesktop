#include "stdafx.h"
#include "SimpleUpdater.h"
#include "cloudveil/response/UpdateResponse.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QProcess>
#include <QtCore/QDirIterator>
#include <QtCore/QTemporaryDir>

#include <cstdio>

#include "ui/boxes/confirm_box.h"
#include "logs.h"
#include "settings.h"
//one day
#define UPDATE_PERIOD_MS 86400000 

// The update stream is chosen by the architecture this binary was BUILT
// for (compile time), not the machine's: an x64 Windows build running
// under emulation on an ARM PC keeps asking for x64. On macOS each slice
// of the universal app is compiled separately, so Apple silicon users of
// the universal build move to the arm64-only stream on their next update.
#define REQUEST_URL_BASE "https://messenger.cloudveil.org/api/v1/messenger/updates/"

#ifdef Q_OS_MAC
	#ifdef Q_PROCESSOR_ARM_64
		#define REQUEST_URL REQUEST_URL_BASE "macos/arm64?current_version=%1"
	#else
		#define REQUEST_URL REQUEST_URL_BASE "macos/universal?current_version=%1"
	#endif
#endif

#ifdef Q_OS_WIN
	#ifdef Q_PROCESSOR_ARM_64
		#define REQUEST_URL REQUEST_URL_BASE "windows/arm64?current_version=%1"
	#else
		#define REQUEST_URL REQUEST_URL_BASE "windows/x64?current_version=%1"
	#endif
#endif

#ifdef Q_OS_LINUX
	#define REQUEST_URL REQUEST_URL_BASE "linux?current_version=%1"
#endif

SimpleUpdater::SimpleUpdater(QObject *parent) : QObject(parent), manager(this)
{
	lastUpdateCheck = 0;
	versionNumber = AppVersion;

	connect(&timer, SIGNAL(timeout()), SLOT(doServerRequest()));
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestDone(QNetworkReply*)));
}


void SimpleUpdater::startUpdateChecking(int currentVersionNumber)
{
	this->versionNumber = currentVersionNumber;
	timer.stop();
	timer.setSingleShot(true);
	timer.start(2000);
}

void SimpleUpdater::downloadUpdate() {
	QNetworkRequest request(lastResponse.url);
	manager.get(request);
}

bool SimpleUpdater::fail(const QString &error) {
	lastError = error;
	LOG(("CloudVeil Update Error: %1").arg(error));
	return false;
}

bool SimpleUpdater::startUpdateProcess() {
	lastError = QString();
	const auto filePath = downloadedFile.fileName();
#if defined Q_OS_WIN
	// The download is the Inno Setup installer; it closes this app itself.
	if (!QProcess::startDetached(filePath, QStringList())) {
		return fail("Could not start the update installer.");
	}
	return true;
#elif defined Q_OS_MAC
	// The download is a disk image. A .dmg can't be executed, so open it in
	// Finder, which shows the drag-to-Applications window. The app quits so
	// the bundle can be replaced.
	if (!QProcess::startDetached(u"/usr/bin/open"_q, { filePath })) {
		return fail("Could not open the downloaded disk image.");
	}
	return true;
#else
	return installLinuxUpdate(filePath);
#endif
}

#if !defined Q_OS_WIN && !defined Q_OS_MAC
// The download is a .tar.xz holding CloudVeilMessenger/cloudveil-messenger.
// Unpack it and replace the running binary in place (rename is atomic and
// safe while the old binary is running); the caller then restarts the app.
bool SimpleUpdater::installLinuxUpdate(const QString &archivePath) {
	QTemporaryDir unpackDir(QDir::tempPath() + u"/cvm_update_XXXXXX"_q);
	if (!unpackDir.isValid()) {
		return fail("Could not create a temporary folder for the update.");
	}
	QProcess tar;
	tar.start(u"tar"_q, { u"-xJf"_q, archivePath, u"-C"_q, unpackDir.path() });
	if (!tar.waitForFinished(120000)
		|| tar.exitStatus() != QProcess::NormalExit
		|| tar.exitCode() != 0) {
		return fail("Could not unpack the update ("
			+ QString::fromLocal8Bit(tar.readAllStandardError()).trimmed()
			+ ").");
	}
	auto found = QDirIterator(
		unpackDir.path(),
		{ u"cloudveil-messenger"_q },
		QDir::Files,
		QDirIterator::Subdirectories);
	if (!found.hasNext()) {
		return fail("The update package does not contain the app.");
	}
	const auto newBinary = found.next();
	{
		auto f = QFile(newBinary);
		if (!f.open(QIODevice::ReadOnly) || f.read(4) != "\x7F" "ELF") {
			return fail("The update package is not a Linux app.");
		}
	}

	const auto target = cExeDir() + cExeName();
	const auto staged = target + u".new"_q;
	QFile::remove(staged);
	if (!QFile::copy(newBinary, staged)) {
		return fail("Could not write to the folder " + cExeDir()
			+ ". Please download the new version from cloudveil.org.");
	}
	QFile::setPermissions(staged, QFile::permissions(staged)
		| QFileDevice::ReadOwner | QFileDevice::WriteOwner
		| QFileDevice::ExeOwner | QFileDevice::ReadGroup
		| QFileDevice::ExeGroup | QFileDevice::ReadOther
		| QFileDevice::ExeOther);
	if (std::rename(
			QFile::encodeName(staged).constData(),
			QFile::encodeName(target).constData()) != 0) {
		QFile::remove(staged);
		return fail("Could not replace " + target + ".");
	}
	return true;
}
#endif

void SimpleUpdater::doServerRequest() {
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - lastUpdateCheck < UPDATE_PERIOD_MS) {
		return;
	}

	QUrl url(QString(REQUEST_URL).arg(versionNumber));

	QNetworkRequest request(url);

	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");	
	manager.get(request);
}

void SimpleUpdater::networkRequestDone(QNetworkReply *networkReply) {
	bool isUpdateCheckResponse = networkReply->request().header(QNetworkRequest::ContentTypeHeader).toString().contains("/json");
	if (isUpdateCheckResponse) {
		processCheckResponse(networkReply);
	}
	else {
		processDownloadedResponse(networkReply);
	}
	
	networkReply->deleteLater();
}

void SimpleUpdater::processCheckResponse(QNetworkReply* networkReply) {
	auto error = networkReply->error();
	if (error == QNetworkReply::NoError) {
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (httpStatusCode >= 200 && httpStatusCode < 300) {
			QJsonDocument json = QJsonDocument::fromJson(networkReply->readAll());
			if (!json.isEmpty()) {
				QJsonObject jsonObj = json.object();
				lastResponse.readFromJson(jsonObj);

				if (lastResponse.url.size() > 0) {
					this->updateReceived(&lastResponse);
				}
			}
			lastUpdateCheck = QDateTime::currentMSecsSinceEpoch();
		}
	}
	else {
		qDebug() << "errorString: " << networkReply->errorString();
	}
}

void SimpleUpdater::processDownloadedResponse(QNetworkReply* networkReply) {
	auto error = networkReply->error();
	if (error == QNetworkReply::NoError) {
		qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (httpStatusCode >= 200 && httpStatusCode < 300) {

			QByteArray data = networkReply->readAll();
			if (downloadedFile.isOpen()) {
				downloadedFile.close();
			}
			
#ifdef Q_OS_WIN
			QString path = QDir::tempPath() + "/cvm_update_package.exe";
#elif defined(Q_OS_MAC)
			QString path = QDir::tempPath() + "/cvm_update_package.dmg";
#else
			QString path = QDir::tempPath() + "/cvm_update_package";
#endif
			downloadedFile.setFileName(path);
			if (QFile::exists(path)) {
				downloadedFile.remove();
			}
			if (downloadedFile.open(QIODevice::ReadWrite)) {
				if (data.size() != downloadedFile.write(data)) {
					downloadedFile.close();
					this->downloadFinished("Error writing update file.");
					qDebug() << "error: Can't write temp file";
				}
				else {
					downloadedFile.close();
					this->downloadFinished("");
				}
			}
			else {
				this->downloadFinished("Error writing update file.");
				qDebug() << "error: Can't save temp file";
			}
		}
		else {
			this->downloadFinished("Error downloading " + networkReply->errorString());
		}
	}
	else {
		this->downloadFinished("Error downloading " + networkReply->errorString());
		qDebug() << "errorString: " << networkReply->errorString();	
	}
}

SimpleUpdater::~SimpleUpdater()
{
}
