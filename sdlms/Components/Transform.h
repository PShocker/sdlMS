#pragma once

#include "Components/Component.h"
#include "Core/Point.h"

class Transform : public Component {
private:
	Point<float> position;
	float rotation;
	int flip;


public:
	Transform();
	~Transform();

	Point<float> get_position() const;
	float get_rotation() const;

	void set_position(const Point<float>& value);
	void set_rotation(const float& value);

	Transform* get_parent() const;
};
