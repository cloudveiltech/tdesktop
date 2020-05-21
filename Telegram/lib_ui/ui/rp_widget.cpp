// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "ui/rp_widget.h"

#include "base/qt_signal_producer.h"

#include <QtGui/QWindow>
#include <QtGui/QtEvents>

namespace Ui {

void ResizeFitChild(
		not_null<RpWidget*> parent,
		not_null<RpWidget*> child) {
	parent->widthValue(
	) | rpl::start_with_next([=](int width) {
		child->resizeToWidth(width);
	}, child->lifetime());

	child->heightValue(
	) | rpl::start_with_next([=](int height) {
		parent->resize(parent->width(), height);
	}, child->lifetime());
}

rpl::producer<not_null<QEvent*>> RpWidgetMethods::events() const {
	auto &stream = eventStreams().events;
	return stream.events();
}

rpl::producer<QRect> RpWidgetMethods::geometryValue() const {
	auto &stream = eventStreams().geometry;
	return stream.events_starting_with_copy(callGetGeometry());
}

rpl::producer<QSize> RpWidgetMethods::sizeValue() const {
	return geometryValue()
		| rpl::map([](QRect &&value) { return value.size(); })
		| rpl::distinct_until_changed();
}

rpl::producer<int> RpWidgetMethods::heightValue() const {
	return geometryValue()
		| rpl::map([](QRect &&value) { return value.height(); })
		| rpl::distinct_until_changed();
}

rpl::producer<int> RpWidgetMethods::widthValue() const {
	return geometryValue()
		| rpl::map([](QRect &&value) { return value.width(); })
		| rpl::distinct_until_changed();
}

rpl::producer<QPoint> RpWidgetMethods::positionValue() const {
	return geometryValue()
		| rpl::map([](QRect &&value) { return value.topLeft(); })
		| rpl::distinct_until_changed();
}

rpl::producer<int> RpWidgetMethods::leftValue() const {
	return geometryValue()
		| rpl::map([](QRect &&value) { return value.left(); })
		| rpl::distinct_until_changed();
}

rpl::producer<int> RpWidgetMethods::topValue() const {
	return geometryValue()
		| rpl::map([](QRect &&value) { return value.top(); })
		| rpl::distinct_until_changed();
}

rpl::producer<int> RpWidgetMethods::desiredHeightValue() const {
	return heightValue();
}

rpl::producer<bool> RpWidgetMethods::shownValue() const {
	auto &stream = eventStreams().shown;
	return stream.events_starting_with(!callIsHidden());
}

rpl::producer<QRect> RpWidgetMethods::paintRequest() const {
	return eventStreams().paint.events();
}

rpl::producer<> RpWidgetMethods::alive() const {
	return eventStreams().alive.events();
}

rpl::producer<> RpWidgetMethods::windowDeactivateEvents() const {
	const auto window = callGetWidget()->window()->windowHandle();
	Assert(window != nullptr);

	return base::qt_signal_producer(
		window,
		&QWindow::activeChanged
	) | rpl::filter([=] {
		return !window->isActive();
	});
}

rpl::producer<> RpWidgetMethods::macWindowDeactivateEvents() const {
#ifdef Q_OS_MAC
	return windowDeactivateEvents();
#else // Q_OS_MAC
	return rpl::never<rpl::empty_value>();
#endif // Q_OS_MAC
}

rpl::lifetime &RpWidgetMethods::lifetime() {
	return _lifetime;
}

bool RpWidgetMethods::handleEvent(QEvent *event) {
	Expects(event != nullptr);

	auto streams = _eventStreams.get();
	if (!streams) {
		return eventHook(event);
	}
	auto that = QPointer<QObject>();
	if (streams->events.has_consumers()) {
		that = callCreateWeak();
		streams->events.fire_copy(event);
		if (!that) {
			return true;
		}
	}
	switch (event->type()) {
	case QEvent::Move:
	case QEvent::Resize:
		if (streams->geometry.has_consumers()) {
			if (!that) {
				that = callCreateWeak();
			}
			streams->geometry.fire_copy(callGetGeometry());
			if (!that) {
				return true;
			}
		}
		break;

	case QEvent::Paint:
		if (streams->paint.has_consumers()) {
			if (!that) {
				that = callCreateWeak();
			}
			const auto rect = static_cast<QPaintEvent*>(event)->rect();
			streams->paint.fire_copy(rect);
			if (!that) {
				return true;
			}
		}
		break;
	}

	return eventHook(event);
}

RpWidgetMethods::Initer::Initer(QWidget *parent) {
	parent->setGeometry(0, 0, 0, 0);
}

void RpWidgetMethods::visibilityChangedHook(bool wasVisible, bool nowVisible) {
	if (nowVisible != wasVisible) {
		if (auto streams = _eventStreams.get()) {
			streams->shown.fire_copy(nowVisible);
		}
	}
}

auto RpWidgetMethods::eventStreams() const -> EventStreams& {
	if (!_eventStreams) {
		_eventStreams = std::make_unique<EventStreams>();
	}
	return *_eventStreams;
}

} // namespace Ui
