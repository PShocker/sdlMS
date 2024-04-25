#pragma once

#include "Components/Component.h"
#include "Core/Vector2.h"

class Transform : public Component {
private:
	Vector2 position;
	float rotation;
	Vector2 scale;

	Transform* parent;

public:
	Transform();
	~Transform();

	Vector2 get_position() const;
	Vector2 get_position_from_parent() const;
	float get_rotation() const;
	Vector2 get_scale() const;

	void set_position(const Vector2& value);
	void set_rotation(const float& value);
	void set_scale(const Vector2& value);

	Transform* get_parent() const;
	void set_parent(Transform* value);
};
