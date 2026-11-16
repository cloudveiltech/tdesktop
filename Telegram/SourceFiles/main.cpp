/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
//CloudVeil start
#define SENTRY_BUILD_STATIC 1
//CloudVeil end
#include "core/launcher.h"
//CloudVeil start
#if defined(DESKTOP_APP_USE_SENTRY) && !defined(_DEBUG)
#include "cloudveil/sentry/include/sentry.h"
#endif
//CloudVeil end


extern "C" {
#include <openssl/crypto.h>
} // extern "C"

int main(int argc, char *argv[]) {
	// OpenSSL's own atexit handler runs OPENSSL_cleanup() on the main thread
	// and frees the library globals while detached background tasks can still
	// be inside OpenSSL, so an ordinary quit can fault on a worker thread.
	// Suppressing that registration leaves the library state alive until the
	// process exits, which is the trade we want: the OS reclaims it anyway.
	OPENSSL_init_crypto(OPENSSL_INIT_NO_ATEXIT, nullptr);
	//CloudVeil start
#if defined(DESKTOP_APP_USE_SENTRY) && !defined(_DEBUG)
		sentry_options_t * options = sentry_options_new();
#ifdef __APPLE__
	sentry_options_set_dsn(options, "https://5c646a91dfb14b52a08d37dc6c72e045@o1077369.ingest.sentry.io/6130964");
	sentry_options_set_database_path(options, "/tmp/sentry-native");
#else
	sentry_options_set_dsn(options, "https://635d4ad185454736b82a72613c20480f@o1077369.ingest.sentry.io/6130963");
#endif
	sentry_init(options);
#endif

	const auto launcher = Core::Launcher::Create(argc, argv);
	auto res = launcher ? launcher->exec() : 1;
#if defined(DESKTOP_APP_USE_SENTRY) && !defined(_DEBUG)
	sentry_close();
#endif

	//CloudVeil end

	return res;
}
