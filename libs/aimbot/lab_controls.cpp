#include "game.h"
#include "lab/aimbot.h"

VAR(0, lab_aim, 0, 0, 2);
FVAR(0, lab_aim_fov, 0, 25, 180);
FVAR(0, lab_aim_speed, 0, 360, 10000);
FVAR(0, lab_aim_ease, 0, 0, 100);
VAR(0, lab_aim_reaction, 0, 0, 5000);
VAR(0, lab_aim_origin, 0, 0, 1);

static bool aim_held = false;
ICOMMAND(0, lab_aim_key, "D", (int *down), aim_held = *down != 0);

extern "C" void lab_aimbot_update(void)
{
    static aimbot::controller aim;
    aimbot::settings config;
    if(lab_aim == 1) config.aim_mode = aimbot::mode::snap;
    else if(lab_aim == 2) config.aim_mode = aimbot::mode::smooth;
    if(lab_aim_origin == 1) config.aim_origin = aimbot::origin::muzzle;
    config.fov = lab_aim_fov;
    config.speed = lab_aim_speed;
    config.ease = lab_aim_ease;
    config.reaction_ms = lab_aim_reaction;

    aimmath::angles view;
    if(aim.update(*lab_get_snapshot(), config, aim_held, curtime / 1000.0f, view))
        lab_set_view(view.yaw, view.pitch);
}
