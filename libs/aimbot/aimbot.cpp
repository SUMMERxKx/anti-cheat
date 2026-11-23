#include "lab/aimbot.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aimbot {

void controller::reset()
{
    target_id = -1;
    acquired_at = 0;
}

bool controller::update(const lab_snapshot &snapshot, const settings &config,
    bool aim_held, float seconds, aimmath::angles &view)
{
    if(!snapshot.active || config.aim_mode == mode::off || (!aim_held && !snapshot.fire_held))
    {
        reset();
        return false;
    }

    lab_vec3 from = snapshot.eye;
    if(config.aim_origin == origin::muzzle) from = snapshot.muzzle;
    aimmath::angles current{snapshot.yaw, snapshot.pitch};
    aimmath::angles desired;
    int32_t selected_id = -1;
    float best_angle = std::numeric_limits<float>::infinity();
    float best_distance = std::numeric_limits<float>::infinity();
    uint32_t count = std::min(snapshot.enemy_count, uint32_t(LAB_MAX_ENEMIES));

    for(uint32_t i = 0; i < count; ++i)
    {
        const lab_enemy &enemy = snapshot.enemies[i];
        if(!enemy.alive || !enemy.visible) continue;
        float distance = aimmath::distance_squared(from, enemy.head);
        if(!std::isfinite(distance) || distance < 1.0e-30f) continue;
        aimmath::angles target = aimmath::point_to_view(from, enemy.head);
        float angle = aimmath::angular_error(current, target);
        if(!std::isfinite(angle) || angle > config.fov) continue;

        // Keep the current target while it remains alive, visible and inside the cone.
        if(enemy.id == target_id)
        {
            selected_id = enemy.id;
            desired = target;
            break;
        }
        if(angle < best_angle || (angle == best_angle && distance < best_distance))
        {
            best_angle = angle;
            best_distance = distance;
            selected_id = enemy.id;
            desired = target;
        }
    }

    if(selected_id == -1)
    {
        reset();
        return false;
    }
    if(selected_id != target_id)
    {
        target_id = selected_id;
        acquired_at = snapshot.millis;
    }

    if(config.aim_mode == mode::snap)
    {
        view = desired;
        return true;
    }
    if(snapshot.millis - acquired_at < config.reaction_ms) return false;
    if(!std::isfinite(seconds) || seconds <= 0) return false;

    float ease_fraction = 1;
    if(config.ease > 0) ease_fraction = -std::expm1(-config.ease * seconds);
    view = aimmath::turn_toward(current, desired, config.speed * seconds, ease_fraction);
    return true;
}

}
