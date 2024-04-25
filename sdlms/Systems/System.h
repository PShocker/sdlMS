#pragma once

class System {
public:
	System() {}
	virtual ~System() {}
	
	virtual void run(class World& world) = 0;
};
