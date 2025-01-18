#include "DialogHelper.h"
#include "lang_auto.h"
#include "data/data_user.h"
#include "ui/boxes/confirm_box.h"
#include "GlobalSecuritySettings.h"
#include <QtGui/QDesktopServices>

DialogHelper::CheckDialogResult DialogHelper::checkAndShowDialogForbidden(PeerData* peerData, UserData* userData, Window::SessionController *controller) {
	if (!GlobalSecuritySettings::getSettings().isDialogAllowed(peerData)) {
		controller->showPeerHistory(userData->id);
		controller->show(Ui::MakeConfirmBox({
					.text = tr::lng_dialog_forbidden(),
					.confirmed = [=](Fn<void()>&& close) {
						//const auto dialogId = DeserializePeerId(peerData->id.value).value; // this was causing 404s on unblock requests
						const auto dialogId = peerData->id.value;
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