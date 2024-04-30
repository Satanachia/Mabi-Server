#include "pch.h"
#include "Projectile.h"
#include "Room.h"
#include "UtilMath.h"
#include "Creature.h"

Projectile::Projectile()
{
	INIT_TL(Projectile);
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"Projectile\n");
}

Projectile::~Projectile()
{
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"~Projectile\n");
}

void Projectile::Init(const Protocol::PosInfo& dir, CreatureRef owner)
{
	_dir = dir;
	_dir = UtilMath::Normalize(_dir);
	_owner = owner;
}

void Projectile::StartUpdateTick()
{
	ASSERT_CRASH(_owner);

	RoomRef room = GetRoom();
	ASSERT_CRASH(room);
	room->DoASync([this]() {Update(); });
}

void Projectile::Update()
{
	RoomRef room = GetRoom();
	if (nullptr == room)
		return;

	Move();
	Collision();
	SendMovePacket();
	--_lifeCount;

	if (0 == _lifeCount)
	{
		_owner = nullptr;
		Destroy();
		return;
	}
	

	std::weak_ptr<Object> objWeakPtr = weak_from_this();
	room->DoTimer(_tickTime, [objWeakPtr]()
	{
		ObjectRef objPtr = objWeakPtr.lock();
		if (nullptr == objPtr)
			return;

		ProjectileRef projectile = std::static_pointer_cast<Projectile>(objPtr);
		if (true == projectile->IsDeath())
			return;

		projectile->Update();
	});
}

void Projectile::Move()
{
	Protocol::PosInfo& curPos = GetCurrentPos();
	Protocol::PosInfo delta= UtilMath::Multi(_dir, _speed);
	curPos = UtilMath::Plus(curPos, delta, true);
}

void Projectile::Collision()
{
	RoomRef room = GetRoom();
	ASSERT_CRASH(room);

	Vector<CreatureRef> Cretures;
	uint64 ownerID = _owner->GetObjectInfo().object_id();
	const Protocol::PosInfo& curPos = GetCurrentPos();
	
	room->CollisionCircle(curPos, _radius, OUT Cretures, ownerID);
	if (true == Cretures.empty())
		return;
	
	for (CreatureRef Creture : Cretures)
	{
		//근데 owner가 죽으면? 그거에 대한 처리도 해줘야 해 몬스터쪽에서
		Creture->OnDamaged(_owner, AttackDamage::SKILL_ATTACK_WARRIOR, curPos);
	}
}

void Projectile::SendMovePacket()
{
	RoomRef room = GetRoom();
	if (nullptr == room)
		return;
	
	const Protocol::ObjectInfo& objInfo = GetObjectInfo();

	Protocol::S_MOVE movePkt;
	movePkt.set_object_id(objInfo.object_id());
	Protocol::PosInfo curPos = GetCurrentPos();
	curPos.set_z(0.f);		//고민 
	movePkt.mutable_current_pos()->CopyFrom(curPos);
	
	SendBufferRef buffer = ClientPacketHandler::MakeSendBuffer(movePkt);
	room->Broadcast(buffer);
}
