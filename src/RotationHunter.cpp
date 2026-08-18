/*
 * Chasseur — Maîtrise des bêtes (0), Précision (1), Survie (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 */

#include "Rotation.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 HUNTERS_MARK   = 1130;
    constexpr uint32 SERPENT_STING  = 1978;
    constexpr uint32 KILL_SHOT      = 53351;
    constexpr uint32 KILL_COMMAND   = 34026;
    constexpr uint32 ARCANE_SHOT    = 3044;
    constexpr uint32 STEADY_SHOT    = 56641;
    constexpr uint32 STEADY_SHOT_OLD = 34120; // ancien premier rang (bas niveau)
    constexpr uint32 CHIMERA_SHOT   = 53209;
    constexpr uint32 AIMED_SHOT     = 19434;
    constexpr uint32 EXPLOSIVE_SHOT = 53301;
    constexpr uint32 BLACK_ARROW    = 3674;
    constexpr uint32 MULTI_SHOT     = 2643;
    constexpr uint32 VOLLEY         = 1510;
    constexpr uint32 RAPTOR_STRIKE  = 2973;

    bool SteadyShot(Ctx& c)
    {
        if (TryCastRank(c.me, c.target, STEADY_SHOT))
            return true;

        return TryCastRank(c.me, c.target, STEADY_SHOT_OLD);
    }

    void BeastMastery(Ctx& c)
    {
        if (TryCastRank(c.me, c.target, KILL_COMMAND)) return; // hors GCD
        if (c.target->HealthBelowPct(20) && TryCastRank(c.me, c.target, KILL_SHOT)) return;
        if (c.Aoe() && TryCastAt(c.me, c.target, VOLLEY)) return;
        if (KeepAuraOn(c.me, c.target, SERPENT_STING)) return;
        if (c.Aoe() && TryCastRank(c.me, c.target, MULTI_SHOT)) return;
        if (TryCastRank(c.me, c.target, ARCANE_SHOT)) return;
        SteadyShot(c);
    }

    void Marksmanship(Ctx& c)
    {
        if (TryCastRank(c.me, c.target, KILL_COMMAND)) return;
        if (c.target->HealthBelowPct(20) && TryCastRank(c.me, c.target, KILL_SHOT)) return;
        if (c.Aoe() && TryCastAt(c.me, c.target, VOLLEY)) return;
        if (KeepAuraOn(c.me, c.target, SERPENT_STING)) return;
        if (TryCastRank(c.me, c.target, CHIMERA_SHOT)) return; // rafraîchit Morsure de serpent
        if (c.Aoe() && TryCastRank(c.me, c.target, MULTI_SHOT)) return;
        if (TryCastRank(c.me, c.target, AIMED_SHOT)) return;
        if (TryCastRank(c.me, c.target, ARCANE_SHOT)) return;
        SteadyShot(c);
    }

    void Survival(Ctx& c)
    {
        if (TryCastRank(c.me, c.target, KILL_COMMAND)) return;
        if (c.target->HealthBelowPct(20) && TryCastRank(c.me, c.target, KILL_SHOT)) return;
        if (TryCastRank(c.me, c.target, EXPLOSIVE_SHOT)) return; // priorité absolue hors exécution
        if (c.Aoe() && TryCastAt(c.me, c.target, VOLLEY)) return;
        if (KeepAuraOn(c.me, c.target, SERPENT_STING)) return;
        if (KeepAuraOn(c.me, c.target, BLACK_ARROW)) return;
        if (c.Aoe() && TryCastRank(c.me, c.target, MULTI_SHOT)) return;
        if (TryCastRank(c.me, c.target, AIMED_SHOT)) return;
        if (TryCastRank(c.me, c.target, ARCANE_SHOT)) return;
        SteadyShot(c);
    }
}

void Hunter(Ctx& c)
{
    if (!c.target)
        return;

    EngageRanged(c);

    // Marque du chasseur avant tout
    if (KeepAuraOn(c.me, c.target, HUNTERS_MARK))
        return;

    // Trop près pour tirer : Attaque du raptor
    if (c.inMelee)
    {
        if (c.me->GetVictim() != c.target)
            c.me->Attack(c.target, true);

        if (!c.me->GetCurrentSpell(CURRENT_MELEE_SPELL))
            TryCastRank(c.me, c.target, RAPTOR_STRIKE);
        return;
    }

    switch (c.tree)
    {
        case 0:  BeastMastery(c); break;
        case 1:  Marksmanship(c); break;
        default: Survival(c);     break;
    }
}
} // namespace ModRotation
