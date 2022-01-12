/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#define SENTRY_BUILD_STATIC 1

#include "core/launcher.h"
//CloudVeil start
#ifndef _DEBUG
#include "cloudveil/sentry/include/sentry.h"
#endif

//CloudVeil end
int main(int argc, char *argv[]) {
	//CloudVeil start
#ifndef _DEBUG
	sentry_options_t* options = sentry_options_new();
	sentry_options_set_dsn(options, "https://635d4ad185454736b82a72613c20480f@o1077369.ingest.sentry.io/6130963");
	sentry_init(options);
#endif

	const auto launcher = Core::Launcher::Create(argc, argv);
	auto res = launcher ? launcher->exec() : 1;
#ifndef _DEBUG
	sentry_close();
#endif // !DEBUG

	//CloudVeil end
	return res;
}
