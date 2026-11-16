#pragma once
#include "window/window_session_controller.h"
#include <data/data_peer_id.h>

class DialogHelper {
public:
	enum CheckDialogResult {
		APPROVED, BLOCKED, UNKNOWN
	};
public:
	static CheckDialogResult checkAndShowDialogForbidden(PeerData* peerData, UserData* userData, Window::SessionController *controller);
	static BareId getDialogId(PeerData* peer);
	// Channel direct messages (monoforums) follow their parent channel's
	// policy. Returns the parent channel for a monoforum, nullptr if a
	// monoforum's parent is not known yet, and the peer itself otherwise.
	static PeerData* policyPeer(PeerData* peer);
};