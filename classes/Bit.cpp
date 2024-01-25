
#include "Bit.h"
#include "BitHolder.h"


Bit::~Bit()
{
}

BitHolder* Bit::getHolder()
{
	// Look for my nearest ancestor that's a BitHolder:
	for (Entity *layer = getParent(); layer; layer = layer->getParent()) {
		if (layer->getEntityType() == EntityBitHolder)
			return (BitHolder *)layer;
		else if (layer->getEntityType() == EntityBit)
			return nullptr;
	}
	return nullptr;
}


void Bit::setPickedUp(bool up)
{
	if (up != _pickedUp) {
		float opacity = 0.0f;
		float scale = 1.0f;
		float rotation = 0.0f;
		int z;

		if (up) {
			opacity = kPickedUpOpacity;
			z = bitz::kPickupUpZ;
			_restingZ = getLocalZOrder();
			_restingTransform = getRotation();
			scale = kPickedUpScale;
		}
		else {
			opacity = 1.0f;
			z = getLocalZOrder();// _restingZ;
			if (z == bitz::kPickupUpZ) {
				z = _restingZ;
			}
			rotation = _restingTransform;
			scale = 1.0f;
		}
		setScale(scale);					// todo: animate this
		setLocalZOrder(z);
		setOpacity( opacity );
		setRotation( rotation );
		_pickedUp = up;
	}
}

bool Bit::friendly()
{
	return true;
}

bool Bit::unfriendly()
{
	return !friendly();
}

bool Bit::getPickedUp()
{
	return _pickedUp;
}

Player* Bit::getOwner()
{
	return _owner;
}

