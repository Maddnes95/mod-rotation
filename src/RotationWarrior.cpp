/*
 * Guerrier — Armes (0), Fureur (1), Protection (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 */

#include "Rotation.h"

namespace ModRotation
{
namespace
{
    // Premiers rangs (3.3.5a)
    constexpr uint32 REND            = 772;
    constexpr uint32 HEROIC_STRIKE   = 78;
    constexpr uint32 CLEAVE          = 845;
    constexpr uint32 EXECUTE         = 5308;
    constexpr uint32 SLAM            = 1464;
    constexpr uint32 WHIRLWIND       = 1680;
    constexpr uint32 BLOODTHIRST     = 23881;
    constexpr uint32 MORTAL_STRIKE   = 12294;
    constexpr uint32 OVERPOWER       = 7384;
    constexpr uint32 BLADESTORM      = 46924;
    constexpr uint32 SWEEPING_STRIKES = 12328;
    constexpr uint32 SHIELD_BLOCK    = 2565;
    constexpr uint32 SHIELD_SLAM     = 23922;
    constexpr uint32 REVENGE         = 6572;
    constexpr uint32 SHOCKWAVE       = 46968;
    constexpr uint32 DEVASTATE       = 20243;
    constexpr uint32 THUNDER_CLAP    = 6343;

    // Procs
    constexpr uint32 AURA_BLOODSURGE    = 46916; // « Heurtoir ! » (Slam instantané)
    constexpr uint32 AURA_SUDDEN_DEATH  = 52437; // Mort soudaine (Execute autorisé)
    constexpr uint32 AURA_TASTE_OF_BLOOD = 60503; // Goût du sang (Overpower autorisé)

    // Frappe héroïque / Enchaînement : « à la prochaine frappe », seulement
    // au-dessus du seuil de rage pour ne pas affamer le reste de la rotation.
    void RageDump(Ctx& c)
    {
        if (!c.inMelee || c.me->GetCurrentSpell(CURRENT_MELEE_SPELL))
            return;

        if (c.me->GetPower(POWER_RAGE) < config.RageDumpThreshold * 10)
            return;

        if (c.Aoe() && TryCastRank(c.me, c.target, CLEAVE))
            return;

        TryCastRank(c.me, c.target, HEROIC_STRIKE);
    }

    void Arms(Ctx& c)
    {
        if (!c.inMelee)
            return;

        // 1. Pourfendre — entretien du saignement (active Goût du sang)
        if (!IsBleedImmune(c.target) && KeepAuraOn(c.me, c.target, REND))
            return;

        // 2. Exécution — proc Mort soudaine ou cible sous 20 %
        if ((c.me->HasAura(AURA_SUDDEN_DEATH) || c.target->HealthBelowPct(20))
            && TryCastRank(c.me, c.target, EXECUTE))
            return;

        // 3. Fulgurance — sur proc Goût du sang
        if (c.me->HasAura(AURA_TASTE_OF_BLOOD) && TryCastRank(c.me, c.target, OVERPOWER))
            return;

        // 4. AoE : Frappes circulaires puis Tempête de lames
        if (c.Aoe())
        {
            if (TryCast(c.me, c.me, MaxRank(c.me, SWEEPING_STRIKES)))
                return;
            if (TryCastRank(c.me, c.target, BLADESTORM))
                return;
        }

        // 5. Frappe mortelle
        if (TryCastRank(c.me, c.target, MORTAL_STRIKE))
            return;

        // 6. Heurtoir (remplisseur, incantation courte)
        if (TryCastRank(c.me, c.target, SLAM))
            return;

        // 7. Frappe héroïque / Enchaînement
        RageDump(c);
    }

    void Fury(Ctx& c)
    {
        // 1. Soif de sang
        if (c.inMelee && TryCastRank(c.me, c.target, BLOODTHIRST))
            return;

        // 2. Tourbillon — plusieurs ennemis au corps à corps
        if (c.Aoe() && TryCastRank(c.me, c.target, WHIRLWIND))
            return;

        // 3. Heurtoir — uniquement sur proc Déferlement sanguin
        if (c.inMelee && c.me->HasAura(AURA_BLOODSURGE) && TryCastRank(c.me, c.target, SLAM))
            return;

        // 4. Pourfendre
        if (c.inMelee && !IsBleedImmune(c.target) && KeepAuraOn(c.me, c.target, REND))
            return;

        // 5. Exécution — cible sous 20 %
        if (c.inMelee && c.target->HealthBelowPct(20) && TryCastRank(c.me, c.target, EXECUTE))
            return;

        // 6/7. Frappe héroïque / Enchaînement
        RageDump(c);
    }

    void Protection(Ctx& c)
    {
        if (!c.inMelee)
            return;

        // Blocage avec bouclier avant Heurt de bouclier (hors GCD)
        TryCast(c.me, c.me, MaxRank(c.me, SHIELD_BLOCK));

        // 1. Heurt de bouclier
        if (TryCastRank(c.me, c.target, SHIELD_SLAM))
            return;

        // 2. Vengeance
        if (TryCastRank(c.me, c.target, REVENGE))
            return;

        // 3. Onde de choc
        if (TryCastRank(c.me, c.target, SHOCKWAVE))
            return;

        // 4. AoE : Coup de tonnerre
        if (c.Aoe() && TryCastRank(c.me, c.target, THUNDER_CLAP))
            return;

        // 5. Dévaster (remplisseur, empile Fracasser armure)
        if (TryCastRank(c.me, c.target, DEVASTATE))
            return;

        // 6. Frappe héroïque / Enchaînement
        RageDump(c);
    }
}

void Warrior(Ctx& c)
{
    if (!c.target)
        return;

    EngageMelee(c);

    switch (c.tree)
    {
        case 0:  Arms(c);       break;
        case 2:  Protection(c); break;
        default: Fury(c);       break;
    }
}
} // namespace ModRotation
