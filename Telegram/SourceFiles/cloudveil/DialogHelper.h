#pragma once
#include "window/window_session_controller.h"

class DialogHelper {
public:
	enum CheckDialogResult {
		APPROVED, BLOCKED, UNKNOWN
	};
public:
	static CheckDialogResult checkAndShowDialogForbidden(PeerData* peerData, UserData* userData, Window::SessionController *controller);
};