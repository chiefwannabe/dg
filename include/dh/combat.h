#ifndef DH_COMBAT_H
#define DH_COMBAT_H

#include "dh/player.h"
#include "dh/enemy.h"
#include "dh/loot.h"
#include "dh/tilemap.h"
#include <raylib.h>
#include <stdbool.h>

#define SWORD_DAMAGE 1
#define SLIME_MAX_HEALTH 3
#define SLIME_KILL_REWARD 1
#define SLIME_XP_REWARD 10
#define ATTACK_COOLDOWN_TIME 0.35f
#define ATTACK_DURATION_TIME 0.40f

typedef struct {
    bool attack_active;         /* Is attack currently in active hit window */
    Rectangle sword_hitbox;     /* Active sword attack hitbox in world space */
    int damage_dealt_count;     /* Total damage hits dealt during session */
    bool show_combat_debug;     /* Combat debug overlay flag */
} DHCombatSystem;

void dh_combat_init(DHCombatSystem *combat);
void dh_combat_update(DHCombatSystem *combat, DHTestPlayer *player, DHEnemyManager *enemies, DHPickupManager *pickups, float dt);
void dh_combat_draw_debug(const DHCombatSystem *combat, const DHTestPlayer *player, const DHEnemyManager *enemies);

#endif /* DH_COMBAT_H */
