/*
 * Paladin — Sacré (0), Protection (1), Vindicte (2).
 * Priorités d'après les guides Wowhead WotLK Classic (FCFS / 969).
 */

#include "Rotation.h"
#include "SharedDefines.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 SEAL_OF_VENGEANCE  = 31801; // Alliance
    constexpr uint32 SEAL_OF_CORRUPTION = 53736; // Horde
    constexpr uint32 SEAL_OF_COMMAND    = 20375;
    constexpr uint32 SEAL_OF_RIGHTEOUSNESS = 21084;

    constexpr uint32 JUDGEMENT_WISDOM   = 53408;
    constexpr uint32 JUDGEMENT_LIGHT    = 20271;
    constexpr uint32 CRUSADER_STRIKE    = 35395;
    constexpr uint32 DIVINE_STORM       = 53385;
    constexpr uint32 CONSECRATION       = 26573;
    constexpr uint32 EXORCISM           = 879;
    constexpr uint32 HOLY_WRATH         = 2812;
    constexpr uint32 HAMMER_OF_WRATH    = 24275;

    constexpr uint32 HOLY_SHIELD        = 20925;
    constexpr uint32 SHIELD_OF_RIGHT    = 53600;
    constexpr uint32 HAMMER_OF_RIGHT    = 53595;

    constexpr uint32 HOLY_SHOCK         = 20473;
    constexpr uint32 FLASH_OF_LIGHT     = 19750;
    constexpr uint32 HOLY_LIGHT         = 635;
    constexpr uint32 BEACON_OF_LIGHT    = 53563;
    constexpr uint32 SACRED_SHIELD      = 53601;

    // Garde un sceau actif (Vengeance/Corruption, sinon Commandement, sinon Droiture).
    bool KeepSeal(Player* p)
    {
        for (uint32 seal : { SEAL_OF_VENGEANCE, SEAL_OF_CORRUPTION, SEAL_OF_COMMAND, SEAL_OF_RIGHTEOUSNESS })
            if (p->GetAuraOfRankedSpell(seal))
                return false;

        for (uint32 seal : { SEAL_OF_VENGEANCE, SEAL_OF_CORRUPTION, SEAL_OF_COMMAND, SEAL_OF_RIGHTEOUSNESS })
            if (p->HasSpell(seal) && TryCast(p, p, seal))
                return true;

        return false;
    }

    bool Judge(Ctx& c)
    {
        if (TryCastRank(c.me, c.target, JUDGEMENT_WISDOM))
            return true;

        return TryCastRank(c.me, c.target, JUDGEMENT_LIGHT);
    }

    bool UndeadOrDemon(Unit* target)
    {
        uint32 type = target->GetCreatureType();
        return type == CREATURE_TYPE_UNDEAD || type == CREATURE_TYPE_DEMON;
    }

    void Retribution(Ctx& c)
    {
        if (!c.target)
            return;

        EngageMelee(c);
        if (KeepSeal(c.me))
            return;

        if (c.Aoe())
        {
            // AoE : Consécration > Tempête divine > Jugement > Courroux sacré > Croisé
            if (TryCastRank(c.me, c.me, CONSECRATION)) return;
            if (c.inMelee && TryCastRank(c.me, c.target, DIVINE_STORM)) return;
            if (Judge(c)) return;
            if (UndeadOrDemon(c.target) && TryCastRank(c.me, c.me, HOLY_WRATH)) return;
            if (c.inMelee && TryCastRank(c.me, c.target, CRUSADER_STRIKE)) return;
            if (TryCastRank(c.me, c.target, EXORCISM)) return;
            return;
        }

        // Mono-cible : Croisé > Jugement > Tempête divine > Consécration > Exorcisme > Courroux sacré
        if (c.target->HealthBelowPct(20) && TryCastRank(c.me, c.target, HAMMER_OF_WRATH)) return;
        if (c.inMelee && TryCastRank(c.me, c.target, CRUSADER_STRIKE)) return;
        if (Judge(c)) return;
        if (c.inMelee && TryCastRank(c.me, c.target, DIVINE_STORM)) return;
        if (c.inMelee && TryCastRank(c.me, c.me, CONSECRATION)) return;
        if (TryCastRank(c.me, c.target, EXORCISM)) return;
        if (UndeadOrDemon(c.target) && TryCastRank(c.me, c.me, HOLY_WRATH)) return;
    }

    void Protection(Ctx& c)
    {
        if (!c.target)
            return;

        EngageMelee(c);
        if (KeepSeal(c.me))
            return;

        // Rotation 969 : Bouclier sacré en priorité défensive, puis alternance 6s/9s.
        if (KeepSelfBuff(c.me, HOLY_SHIELD)) return;
        if (c.target->HealthBelowPct(20) && TryCastRank(c.me, c.target, HAMMER_OF_WRATH)) return;
        if (c.inMelee && TryCastRank(c.me, c.target, HAMMER_OF_RIGHT)) return;
        if (c.inMelee && TryCastRank(c.me, c.target, SHIELD_OF_RIGHT)) return;
        if (TryCastRank(c.me, c.me, CONSECRATION)) return;
        if (Judge(c)) return;
        if (TryCastRank(c.me, c.me, HOLY_SHIELD)) return;
    }

    void Holy(Ctx& c)
    {
        Unit* ally = c.friendly;
        float pct = ally->GetHealthPct();

        // Entretien : Guide de lumière et Bouclier sacré sur la cible amicale
        if (ally != c.me && c.me->HasSpell(BEACON_OF_LIGHT) && KeepAuraOn(c.me, ally, BEACON_OF_LIGHT))
            return;
        if (c.me->HasSpell(SACRED_SHIELD) && KeepAuraOn(c.me, ally, SACRED_SHIELD))
            return;

        // Soins par gravité
        if (pct < config.HealUrgentPct)
        {
            if (TryCastRank(c.me, ally, HOLY_SHOCK)) return;
            if (TryCastRank(c.me, ally, HOLY_LIGHT)) return;
            TryCastRank(c.me, ally, FLASH_OF_LIGHT);
            return;
        }
        if (pct < config.HealLowPct)
        {
            if (TryCastRank(c.me, ally, HOLY_SHOCK)) return;
            TryCastRank(c.me, ally, FLASH_OF_LIGHT);
            return;
        }
        if (pct < config.HealInjuredPct)
        {
            TryCastRank(c.me, ally, FLASH_OF_LIGHT);
            return;
        }

        // Personne à soigner : contribution légère en dégâts
        if (c.target)
        {
            if (KeepSeal(c.me)) return;
            if (Judge(c)) return;
            TryCastRank(c.me, c.target, EXORCISM);
        }
    }
}

void Paladin(Ctx& c)
{
    switch (c.tree)
    {
        case 0:  Holy(c);        break;
        case 1:  Protection(c);  break;
        default: Retribution(c); break;
    }
}
} // namespace ModRotation
