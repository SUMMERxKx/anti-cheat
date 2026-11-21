#ifndef LAB_API_H
#define LAB_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LAB_MAX_ENEMIES 256

/* Positions use engine world units. Velocity is world units per second. */
/* Angles are degrees. Yaw 0 faces +Y, yaw 90 faces -X, positive pitch looks up. */
typedef struct lab_vec3 {
    float x, y, z;
} lab_vec3;

typedef struct lab_enemy {
    int32_t id;
    int32_t team;
    int32_t alive;
    int32_t visible;
    lab_vec3 head;
    lab_vec3 velocity;
} lab_enemy;

typedef struct lab_snapshot {
    int32_t active;
    uint32_t millis;
    lab_vec3 eye;
    lab_vec3 muzzle;
    float yaw;
    float pitch;
    int32_t weapon;
    int32_t fire_held;
    uint32_t enemy_count;
    lab_enemy enemies[LAB_MAX_ENEMIES];
} lab_snapshot;

/* Main thread only. Borrowed storage lasts until the next game update. */
/* Outside a local offline match this returns an all-zero snapshot. */
const lab_snapshot *lab_get_snapshot(void);

/* Uses the mouse-look angle path. Outside a local offline match it does nothing. */
void lab_set_view(float yaw, float pitch);

#ifdef __cplusplus
}
#endif

#endif
