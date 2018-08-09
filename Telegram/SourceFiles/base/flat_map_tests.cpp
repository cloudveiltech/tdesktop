/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "catch.hpp"

#include "base/flat_map.h"
#include <string>

struct int_wrap {
	int value;
};
struct int_wrap_comparator {
	inline bool operator()(const int_wrap &a, const int_wrap &b) const {
		return a.value < b.value;
	}
};

using namespace std;

TEST_CASE("flat_maps should keep items sorted by key", "[flat_map]") {
	base::flat_map<int, string> v;
	v.emplace(0, "a");
	v.emplace(5, "b");
	v.emplace(4, "d");
	v.emplace(2, "e");

	auto checkSorted = [&] {
		auto prev = v.begin();
		REQUIRE(prev != v.end());
		for (auto i = prev + 1; i != v.end(); prev = i, ++i) {
			REQUIRE(prev->first < i->first);
		}
	};
	REQUIRE(v.size() == 4);
	checkSorted();

	SECTION("adding item puts it in the right position") {
		v.emplace(3, "c");
		REQUIRE(v.size() == 5);
		REQUIRE(v.find(3) != v.end());
		checkSorted();
	}
}

TEST_CASE("flat_maps custom comparator", "[flat_map]") {
	base::flat_map<int_wrap, string, int_wrap_comparator> v;
	v.emplace({ 0 }, "a");
	v.emplace({ 5 }, "b");
	v.emplace({ 4 }, "d");
	v.emplace({ 2 }, "e");

	auto checkSorted = [&] {
		auto prev = v.begin();
		REQUIRE(prev != v.end());
		for (auto i = prev + 1; i != v.end(); prev = i, ++i) {
			REQUIRE(int_wrap_comparator()(prev->first, i->first));
		}
	};
	REQUIRE(v.size() == 4);
	checkSorted();

	SECTION("adding item puts it in the right position") {
		v.emplace({ 3 }, "c");
		REQUIRE(v.size() == 5);
		REQUIRE(v.find({ 3 }) != v.end());
		checkSorted();
	}
}
