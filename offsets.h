#pragma once
#include <cstddef>

namespace offsets {
    constexpr ptrdiff_t dwClientState = 0x59F19C;
    constexpr ptrdiff_t dwClientState_GetLocalPlayer = 0x180;
    constexpr ptrdiff_t dwEntityList = 0x4E051DC;
    constexpr ptrdiff_t dwLocalPlayer = 0xDEF97C;
    constexpr ptrdiff_t dwViewMatrix = 0x4DF6024;
    
    // Netvars
    constexpr ptrdiff_t m_iHealth = 0x100;
    constexpr ptrdiff_t m_vecOrigin = 0x138;
    constexpr ptrdiff_t m_iTeamNum = 0xF4;
    constexpr ptrdiff_t m_lifeState = 0x25F;
    constexpr ptrdiff_t m_bDormant = 0xED;
    constexpr ptrdiff_t m_bSpotted = 0x93D;
}
