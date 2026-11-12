#include "ps4_pad.h"

#include <orbis/Pad.h>
#include <orbis/UserService.h>

#include <cstdint>

#include "../src/platform_paths.h"

namespace {

int32_t g_handle = -1;
bool g_initialised = false;
unsigned g_retry = 0;
bool g_read_error_logged = false;

int32_t open_pad() {
    int32_t user = 1000;  // shadPS4 and most consoles use user 1000.
    if (sceUserServiceInitialize(nullptr) == 0) {
        int32_t initial = -1;
        if (sceUserServiceGetInitialUser(&initial) == 0 && initial >= 0) {
            user = initial;
        }
    }
    const int32_t rc = scePadInit();
    jill_paths::trace("pad: scePadInit rc=0x%x user=%d", rc, user);
    int32_t handle = scePadOpen(user, ORBIS_PAD_PORT_TYPE_STANDARD, 0, nullptr);
    jill_paths::trace("pad: scePadOpen rc=0x%x", handle);
    if (handle <= 0) {
        // Fall back to the system user id; some firmware/shims reject user ids.
        handle = scePadOpen(ORBIS_USER_SERVICE_USER_ID_SYSTEM,
                            ORBIS_PAD_PORT_TYPE_STANDARD, 0, nullptr);
        jill_paths::trace("pad: scePadOpen(system) rc=0x%x", handle);
    }
    return handle;
}

}  // namespace

bool ps4_pad_init() {
    if (!g_initialised) {
        g_initialised = true;
        g_handle = open_pad();
    }
    return g_handle > 0;
}

void ps4_pad_poll(Ps4PadState& out) {
    out = Ps4PadState{};
    if (g_handle <= 0) {
        // Retry periodically so a pad connected after boot still works.
        if (++g_retry % 120 == 0) {
            g_handle = open_pad();
        }
        return;
    }
    OrbisPadData data{};
    const int32_t rc = scePadReadState(g_handle, &data);
    if (rc != 0 || !data.connected) {
        if (!g_read_error_logged) {
            g_read_error_logged = true;
            jill_paths::trace("pad: read failed rc=0x%x connected=%d", rc,
                              static_cast<int>(data.connected));
        }
        return;
    }
    const uint32_t b = data.buttons;
    out.connected = true;
    out.up = (b & ORBIS_PAD_BUTTON_UP) != 0;
    out.down = (b & ORBIS_PAD_BUTTON_DOWN) != 0;
    out.left = (b & ORBIS_PAD_BUTTON_LEFT) != 0;
    out.right = (b & ORBIS_PAD_BUTTON_RIGHT) != 0;
    out.cross = (b & ORBIS_PAD_BUTTON_CROSS) != 0;
    out.circle = (b & ORBIS_PAD_BUTTON_CIRCLE) != 0;
    out.square = (b & ORBIS_PAD_BUTTON_SQUARE) != 0;
    out.triangle = (b & ORBIS_PAD_BUTTON_TRIANGLE) != 0;
    out.l1 = (b & ORBIS_PAD_BUTTON_L1) != 0;
    out.r1 = (b & ORBIS_PAD_BUTTON_R1) != 0;
    out.l2 = (b & ORBIS_PAD_BUTTON_L2) != 0;
    out.r2 = (b & ORBIS_PAD_BUTTON_R2) != 0;
    out.options = (b & ORBIS_PAD_BUTTON_OPTIONS) != 0;
    out.l3 = (b & ORBIS_PAD_BUTTON_L3) != 0;
    out.r3 = (b & ORBIS_PAD_BUTTON_R3) != 0;
    out.touch = (b & ORBIS_PAD_BUTTON_TOUCH_PAD) != 0;

    // Sticks report 0..255 with 128 centred.
    out.stick_x = (static_cast<int>(data.leftStick.x) - 128) / 127.0f;
    out.stick_y = (static_cast<int>(data.leftStick.y) - 128) / 127.0f;
}

void ps4_pad_shutdown() {
    if (g_handle > 0) {
        scePadClose(g_handle);
        g_handle = -1;
    }
}
