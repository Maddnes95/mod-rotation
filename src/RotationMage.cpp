/*
 * Mage — Arcanes (0), Feu (1), Givre (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 */

#include "Rotation.h"
#include "SpellAuras.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 ARCANE_BLAST    = 30451;
    constexpr uint32 ARCANE_MISSILES = 5143;
    constexpr uint32 ARCANE_BARRAGE  = 44425;
    constexpr uint32 ARCANE_EXPLOSION = 1449;
    constexpr uint32 FIREBALL        = 133;
    constexpr uint32 PYROBLAST       = 11366;
    constexpr uint32 LIVING_BOMB     = 44457;
    constexpr uint32 FIRE_BLAST      = 2136;
    constexpr uint32 FLAMESTRIKE     = 2120;
    constexpr uint32 FROSTBOLT       = 116;
    constexpr uint32 FROSTFIRE_BOLT  = 44614;
    constexpr uint32 DEEP_FREEZE     = 44572;
    constexpr uint32 ICE_LANCE       = 30455;
    constexpr uint32 BLIZZARD        = 10;
    constexpr uint32 SUMMON_WATER_ELEMENTAL = 31687;

    constexpr uint32 AURA_AB_STACK        = 36032; // Déflagration des arcanes (débuff personnel empilable)
    constexpr uint32 AURA_MISSILE_BARRAGE = 44401; // Barrage de projectiles
    constexpr uint32 AURA_HOT_STREAK      = 48108; // Chaleur continue (Pyroblast instantané)
    constexpr uint32 AURA_BRAIN_FREEZE    = 57761; // « Boule de feu ! » (Gel de la pensée)
    constexpr uint32 AURA_FINGERS_OF_FROST = 44544; // Doigts de givre

    void Arcane(Ctx& c)
    {
        if (c.Aoe())
        {
            if (c.me->IsWithinMeleeRange(c.target) && TryCastRank(c.me, c.me, ARCANE_EXPLOSION)) return;
            if (TryCastAt(c.me, c.target, BLIZZARD)) return;
        }

        // Missiles des arcanes sur proc Barrage ou à 4 charges de Déflagration
        Aura* stack = c.me->GetAura(AURA_AB_STACK);
        bool dump = c.me->HasAura(AURA_MISSILE_BARRAGE) || (stack && stack->GetStackAmount() >= 4);

        if (dump && TryCastRank(c.me, c.target, ARCANE_MISSILES))
            return;

        if (TryCastRank(c.me, c.target, ARCANE_BLAST)) return;
        if (TryCastRank(c.me, c.target, ARCANE_BARRAGE)) return;
        TryCastRank(c.me, c.target, FIREBALL); // bas niveau, avant Déflagration
    }

    void Fire(Ctx& c)
    {
        // Pyroblast instantané sur Chaleur continue
        if (c.me->HasAura(AURA_HOT_STREAK) && TryCastRank(c.me, c.target, PYROBLAST))
            return;

        if (c.Aoe() && TryCastAt(c.me, c.target, FLAMESTRIKE))
            return;

        // Bombe vivante entretenue
        if (c.me->HasSpell(LIVING_BOMB) && KeepAuraOn(c.me, c.target, LIVING_BOMB))
            return;

        if (TryCastRank(c.me, c.target, FIREBALL)) return;
        TryCastRank(c.me, c.target, FIRE_BLAST);
    }

    void Frost(Ctx& c)
    {
        // Elémentaire d'eau si disponible
        if (c.me->HasSpell(SUMMON_WATER_ELEMENTAL) && !c.me->GetPet()
            && TryCast(c.me, c.me, SUMMON_WATER_ELEMENTAL))
            return;

        if (c.Aoe() && TryCastAt(c.me, c.target, BLIZZARD))
            return;

        // Doigts de givre : Sarcophage de glace, sinon Javelot de glace
        if (c.me->HasAura(AURA_FINGERS_OF_FROST))
        {
            if (TryCastRank(c.me, c.target, DEEP_FREEZE)) return;
            if (TryCastRank(c.me, c.target, ICE_LANCE)) return;
        }

        // « Boule de feu ! » : Sort de givre et de feu instantané
        if (c.me->HasAura(AURA_BRAIN_FREEZE) && TryCastRank(c.me, c.target, FROSTFIRE_BOLT))
            return;

        TryCastRank(c.me, c.target, FROSTBOLT);
    }
}

void Mage(Ctx& c)
{
    if (!c.target)
        return;

    switch (c.tree)
    {
        case 0:  Arcane(c); break;
        case 1:  Fire(c);   break;
        default: Frost(c);  break;
    }
}
} // namespace ModRotation
