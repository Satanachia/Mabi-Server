#include "pch.h"
#include "FieldItem.h"
#include "ObjectUtils.h"

FieldItem::FieldItem()
{
	INIT_TL(FieldItem);
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"FieldItem\n");
}

FieldItem::~FieldItem()
{
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"~FieldItem\n");
}


void FieldItem::Init(int32 count)
{
	uint64 objectID = GetObjectInfo().object_id();
	Protocol::ItemType type = static_cast<Protocol::ItemType>(ObjectUtils::FindSecondType(objectID));
	_info.set_type(type);
	_info.set_count(count);
}
