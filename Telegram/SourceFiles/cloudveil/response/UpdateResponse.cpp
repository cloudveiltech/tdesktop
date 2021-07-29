#include "stdafx.h"
#include "UpdateResponse.h"


UpdateResponse::UpdateResponse()
{
}


void UpdateResponse::readFromJson(QJsonObject &jsonObject) {
	if (jsonObject.contains("url") && jsonObject["url"].isString())
	{
		url = jsonObject["url"].toString();
	}
	if (jsonObject.contains("message") && jsonObject["message"].isString())
	{
		message = jsonObject["message"].toString();
	}

}

UpdateResponse::~UpdateResponse()
{
}
