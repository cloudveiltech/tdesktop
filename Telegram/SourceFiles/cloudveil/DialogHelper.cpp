#include "DialogHelper.h"
#include "lang_auto.h"
#include "data/data_user.h"
#include "ui/boxes/confirm_box.h"
#include "GlobalSecuritySettings.h"
#include <QtGui/QDesktopServices>

DialogHelper::CheckDialogResult DialogHelper::checkAndShowDialogForbidden(PeerData* peerData, UserData* userData, Window::SessionController *controller) {
	if (!GlobalSecuritySettings::getSettings().isDialogAllowed(peerData)) {
        // Kenji: this line below makes the user lose the current view context
        // It usually opens the first chat or the saved chat, so I commented it out.
        // controller->showPeerHistory(userData->id);
		controller->show(Ui::MakeConfirmBox({
					.text = tr::lng_dialog_forbidden(),
					.confirmed = [=](Fn<void()>&& close) {
                        const auto dialogId = getDialogId(peerData);
						QString url = QString("https://messenger.cloudveil.org/unblock/%1/%2")
							.arg(QString::number(userData->id.value), QString::number(dialogId));

						QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
					},
					.confirmText = tr::lng_contact(),

			})
		);
		return DialogHelper::CheckDialogResult::BLOCKED;
	}
	if (!GlobalSecuritySettings::getSettings().isDialogSecured(peerData)) {
		GlobalSecuritySettings::getInstance()->addAdditionalDataToRequest(peerData);
		GlobalSecuritySettings::getInstance()->updateFromServer();
		return DialogHelper::CheckDialogResult::UNKNOWN;
	}
	return DialogHelper::CheckDialogResult::APPROVED;
}

BareId DialogHelper::getDialogId(PeerData* peer)
{
	if (peer->isChat()) {
		return peerToChat(peer->id).bare;
	}
	else if (peer->isChannel()) {
		return peerToChannel(peer->id).bare;
	}
	else if (peer->isUser()) {
		return peerToUser(peer->id).bare;
	}
	return DeserializePeerId(peer->id.value).value;	
}
