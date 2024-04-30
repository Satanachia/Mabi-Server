#pragma once
#include "Object.h"

class FieldItem : public Object
{
	friend class ObjectUtils;

public:
	FieldItem();
	~FieldItem();

	FieldItem(const FieldItem& _Other) = delete;
	FieldItem(FieldItem&& _Other) noexcept = delete;
	FieldItem& operator=(const FieldItem& _Other) = delete;
	FieldItem& operator=(const FieldItem&& _Other) noexcept = delete;
	
	const Protocol::ItemInfo& GetItemInfo() { return _info; }

private:
	void Init(int32 count = 1);

private:
	Protocol::ItemInfo _info;
};

