#pragma once
#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include <stdio.h>
#include <glm/glm.hpp>
#include "system.h"

using namespace glm;

namespace Lynx {

class PhysicsSystem : public ECS::System {
	
	public:
		void Init();
		void Update();
		vec3 gravityForce = vec3(0, -1.0f,0);
};

}

#endif