/*
 * Chaman — Elémentaire (0), Amélioration (1), Restauration (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 * (La gestion des totems n'est pas automatisée.)
 */

#include "Rotation.h"
#include "SpellAuras.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 FLAME_SHOCK     = 8050;
    constexpr uint32 EARTH_SHOCK     = 8042;
    constexpr uint32 LAVA_BURST      = 51505;
    constexpr uint32 LIGHTNING_BOLT  = 403;
    constexpr uint32 CHAIN_LIGHTNING = 421;
    constexpr uint32 STORMSTRIKE     = 17364;
    constexpr uint32 LAVA_LASH       = 60103;
    constexpr uint32 LIGHTNING_SHIELD = 324;
    constexpr uint32 WATER_SHIELD    = 52127;
    constexpr uint32 EARTH_SHIELD    = 974;
    constexpr uint32 RIPTIDE         = 61295;
    constexpr uint32 LESSER_HEALING_WAVE = 8004;
    constexpr uint32 HEALING_WAVE    = 331;
    constexpr uint32 CHAIN_HEAL      = 1064;

    constexpr uint32 AURA_MAELSTROM  = 53817; // Arme du Maelström (x5 = sort instantané)

    bool MaelstromReady(Player* p)
    {
        Aura* aura = p->GetAura(AURA_MAELSTROM);
        return aura && aura->GetStackAmount() >= 5;
    }

    void Elemental(Ctx& c)
    {
        if (KeepSelfBuff(c.me, WATER_SHIELD))
            return;

        if (!c.target)
            return;

        // Horion de flammes entretenu, puis Salve de lave (crit garanti avec le horion)
        if (KeepAuraOn(c.me, c.target, FLAME_SHOCK)) return;
        if (TryCastRank(c.me, c.target, LAVA_BURST)) return;
        if (c.Aoe() && TryCastRank(c.me, c.target, CHAIN_LIGHTNING)) return;
        TryCastRank(c.me, c.target, LIGHTNING_BOLT);
    }

    void Enhancement(Ctx& c)
    {
        if (KeepSelfBuff(c.me, LIGHTNING_SHIELD))
            return;

        if (!c.target)
            return;

        // Eclair (instantané) avec 5 charges d'Arme du Maelström
        if (MaelstromReady(c.me))
        {
            if (c.Aoe() && TryCastRank(c.me, c.target, CHAIN_LIGHTNING)) return;
            if (TryCastRank(c.me, c.target, LIGHTNING_BOLT)) return;
        }

        if (!c.inMelee)
            return;

        if (TryCastRank(c.me, c.target, STORMSTRIKE)) return;
        if (KeepAuraOn(c.me, c.target, FLAME_SHOCK)) return;
        if (TryCastRank(c.me, c.target, EARTH_SHOCK)) return;
        TryCastRank(c.me, c.target, LAVA_LASH);
    }

    void Restoration(Ctx& c)
    {
        if (KeepSelfBuff(c.me, WATER_SHIELD))
            return;

        Unit* ally = c.friendly;
        float pct = ally->GetHealthPct();

        // Bouclier de terre entretenu sur l'allié ciblé
        if (ally != c.me && c.me->HasSpell(EARTH_SHIELD) && KeepAuraOn(c.me, ally, EARTH_SHIELD))
            return;

        if (pct < config.HealUrgentPct)
        {
            if (TryCastRank(c.me, ally, RIPTIDE)) return;
            if (TryCastRank(c.me, ally, LESSER_HEALING_WAVE)) return;
            TryCastRank(c.me, ally, HEALING_WAVE);
            return;
        }
        if (pct < config.HealLowPct)
        {
            if (TryCastRank(c.me, ally, RIPTIDE)) return;
            if (TryCastRank(c.me, ally, CHAIN_HEAL)) return;
            TryCastRank(c.me, ally, LESSER_HEALING_WAVE);
            return;
        }
        if (pct < config.HealInjuredPct)
        {
            if (TryCastRank(c.me, ally, RIPTIDE)) return;
            TryCastRank(c.me, ally, LESSER_HEALING_WAVE);
            return;
        }

        // Personne à soigner : contribution légère en dégâts
        if (c.target)
        {
            if (KeepAuraOn(c.me, c.target, FLAME_SHOCK)) return;
            TryCastRank(c.me, c.target, LIGHTNING_BOLT);
        }
    }
}

void Shaman(Ctx& c)
{
    if (c.target && c.tree == 1)
        EngageMelee(c);

    switch (c.tree)
    {
        case 0:  Elemental(c);   break;
        case 1:  Enhancement(c); break;
        default: Restoration(c); break;
    }
}
} // namespace ModRotation
