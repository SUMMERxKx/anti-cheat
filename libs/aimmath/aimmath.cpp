#include "lab/aimmath.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aimmath {

static constexpr float radians = std::numbers::pi_v<float> / 180.0f;

float distance_squared(lab_vec3 origin, lab_vec3 point)
{
    float x = point.x - origin.x;
    float y = point.y - origin.y;
    float z = point.z - origin.z;
    return x*x + y*y + z*z;
}

angles point_to_view(lab_vec3 origin, lab_vec3 point)
{
    float x = point.x - origin.x;
    float y = point.y - origin.y;
    float z = point.z - origin.z;
    float length_squared = x*x + y*y + z*z;
    if(length_squared < 1.0e-30f) return {};

    // Same convention and zero threshold as shared/geom.cpp vectoyawpitch().
    angles result;
    result.yaw = -std::atan2(x, y) / radians;
    result.pitch = std::asin(std::clamp(z / std::sqrt(length_squared), -1.0f, 1.0f)) / radians;
    return result;
}

float angular_error(angles view, angles target)
{
    // atan2 keeps small errors useful when a dot product would round to one.
    float yaw = std::remainder(target.yaw - view.yaw, 360.0f) * radians;
    float pitch = view.pitch * radians;
    float target_pitch = target.pitch * radians;
    float x = -std::sin(yaw) * std::cos(target_pitch);
    float y = std::cos(yaw) * std::cos(target_pitch);
    float z = std::sin(target_pitch);
    float cross_x = std::cos(pitch)*z - std::sin(pitch)*y;
    float cross_y = std::sin(pitch)*x;
    float cross_z = -std::cos(pitch)*x;
    float cross_length = std::sqrt(cross_x*cross_x + cross_y*cross_y + cross_z*cross_z);
    float dot = std::cos(pitch)*y + std::sin(pitch)*z;
    return std::atan2(cross_length, dot) / radians;
}

angles turn_toward(angles view, angles target, float max_degrees, float ease_fraction)
{
    float yaw = std::remainder(target.yaw - view.yaw, 360.0f);
    float pitch = target.pitch - view.pitch;
    float travel = std::hypot(yaw, pitch);
    if(travel == 0) return target;

    // The yaw/pitch path bounds the angle travelled on the view sphere.
    float fraction = std::min(std::max(max_degrees, 0.0f) / travel,
        std::clamp(ease_fraction, 0.0f, 1.0f));
    angles result;
    result.yaw = std::remainder(view.yaw + yaw*fraction, 360.0f);
    result.pitch = view.pitch + pitch*fraction;
    return result;
}

}
