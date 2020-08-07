/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "core/launcher.h"
#define SENTRY_BUILD_STATIC 1
#include "cloudveil/sentry/include/sentry.h"

void logger_func(sentry_level_t level, const char* message, va_list args) {
	char buf[1024]{};
	vsprintf(buf, message, args);

	qDebug() << buf << "\n";
}

int main(int argc, char *argv[]) {
	//CloudVeil start
	sentry_options_t* options = sentry_options_new();

	sentry_options_set_debug(options, 1);
	QString s = QString(argv[0]);
	QString exePath = s.mid(0, s.lastIndexOf("\\"));

	s = exePath +"\\crashpad_handler.exe";
	sentry_options_set_handler_path(options, s.toStdString().c_str());

	s = exePath + "\\.sentry-native";
	sentry_options_set_database_path(options, ".sentry-native");
	
	sentry_options_set_logger(options, logger_func);

	sentry_options_set_release(options, AppVersionStr);
#ifdef Q_OS_WIN
	sentry_options_set_dsn(options, "https://c75cbc48ea40439c88d9b54129c8bbd6@sentry.cloudveil.org/21");	
#else
#ifdef Q_OS_MAC
	sentry_options_set_dsn(options, "https://8de23b56a86a4701b95664b253db89f5:3d29504d0be64c948225a253a7cd7347@sentry.cloudveil.org/20");
#else 
#endif
	sentry_options_set_dsn(options, "https://322e71e0f8604a949fa0c0d6e14f07b9:f11fd49ebb644ee695acb06af12b32c3@sentry.cloudveil.org/22");
#endif
	sentry_init(options);	

	const auto launcher = Core::Launcher::Create(argc, argv);
	if (launcher) {
		launcher->exec();
	}

	sentry_shutdown();
	//CloudVeil end
	return 1;
}
