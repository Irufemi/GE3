#pragma once

#include "../Vector3.h"
#include "../Vector4.h"
#include "../Transform.h"

struct Particle {
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;
};

