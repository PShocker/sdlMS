#include "Component.h"

Component::Component() {}
Component::~Component() {}


Entity* Component::get_owner() const {
	return owner;
}

void Component::set_owner(Entity* value) {
	owner = value;
}
