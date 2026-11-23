#ifndef LAB_AIMBOT_H
#define LAB_AIMBOT_H

#include "lab/aimmath.h"

namespace aimbot {

enum class mode { off, snap, smooth };
enum class origin { eye, muzzle };

struct settings {
    mode aim_mode = mode::off;
    origin aim_origin = origin::eye;
    float fov = 25;
    float speed = 360;
    float ease = 0;
    uint32_t reaction_ms = 0;
};

class controller {
public:
    // Returns a view only while aiming in an active lab snapshot.
    bool update(const lab_snapshot &snapshot, const settings &config,
        bool aim_held, float seconds, aimmath::angles &view);
    void reset();

private:
    int32_t target_id = -1;
    uint32_t acquired_at = 0;
    uint32_t previous_millis = 0;
};

}

#endif
