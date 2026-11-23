#ifndef LAB_AIMMATH_H
#define LAB_AIMMATH_H

#include "lab/lab_api.h"

namespace aimmath {

struct angles {
    float yaw = 0;
    float pitch = 0;
};

float distance_squared(lab_vec3 origin, lab_vec3 point);
angles point_to_view(lab_vec3 origin, lab_vec3 point);
float angular_error(angles view, angles target);
angles turn_toward(angles view, angles target, float max_degrees, float ease_fraction);

}

#endif
