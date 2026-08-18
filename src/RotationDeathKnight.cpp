/*
 * Chevalier de la mort — Sang (0), Givre (1), Impie (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 * Les coûts en runes sont vérifiés par le cœur au moment du cast.
 */

#include "Rotation.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 ICY_TOUCH      = 45477;
    constexpr uint32 PLAGUE_STRIKE  = 45462;
    constexpr uint32 BLOOD_STRIKE   = 45902;
    constexpr uint32 HEART_STRIKE   = 55050;
    constexpr uint32 DEATH_STRIKE   = 49998;
    constexpr uint32 OBLITERATE     = 49020;
    constexpr uint32 SCOURGE_STRIKE = 55090;
    constexpr uint32 FROST_STRIKE   = 49143;
    constexpr uint32 DEATH_COIL     = 47541;
    constexpr uint32 HOWLING_BLAST  = 49184;
    constexpr uint32 BLOOD_BOIL     = 48721;
    constexpr uint32 PESTILENCE     = 50842;
    constexpr uint32 DEATH_AND_DECAY = 43265;
    constexpr uint32 RUNE_STRIKE    = 56815;

    constexpr uint32 DISEASE_FROST_FEVER  = 55095; // Fièvre de givre
    constexpr uint32 DISEASE_BLOOD_PLAGUE = 55078; // Peste de sang
    constexpr uint32 AURA_FREEZING_FOG    = 59052; // Brouillard givrant (Rime) : Rafale hurlante gratuite

    bool DiseasesUp(Ctx& c)
    {
        return c.target->HasAura(DISEASE_FROST_FEVER, c.me->GetGUID())
            && c.target->HasAura(DISEASE_BLOOD_PLAGUE, c.me->GetGUID());
    }

    // Applique les maladies manquantes. Retourne true si un cast est parti.
    bool ApplyDiseases(Ctx& c)
    {
        if (!c.target->HasAura(DISEASE_FROST_FEVER, c.me->GetGUID())
            && TryCastRank(c.me, c.target, ICY_TOUCH))
            return true;

        if (!c.target->HasAura(DISEASE_BLOOD_PLAGUE, c.me->GetGUID())
            && c.inMelee && TryCastRank(c.me, c.target, PLAGUE_STRIKE))
            return true;

        return false;
    }

    void Blood(Ctx& c)
    {
        if (ApplyDiseases(c))
            return;

        if (c.Aoe() && DiseasesUp(c))
        {
            if (TryCastAt(c.me, c.target, DEATH_AND_DECAY)) return;
            if (TryCastRank(c.me, c.target, PESTILENCE)) return;
            if (TryCastRank(c.me, c.target, BLOOD_BOIL)) return;
        }

        if (!c.inMelee)
            return;

        // Frappe de mort (soin + dégâts), puis Frappe du cœur / de sang
        if (TryCastRank(c.me, c.target, DEATH_STRIKE)) return;
        if (TryCastRank(c.me, c.target, HEART_STRIKE)) return;
        if (TryCastRank(c.me, c.target, BLOOD_STRIKE)) return;

        // Décharges : Frappe runique (après esquive/parade), sinon Voile mortel
        if (TryCastRank(c.me, c.target, RUNE_STRIKE)) return;
        TryCastRank(c.me, c.target, DEATH_COIL);
    }

    void Frost(Ctx& c)
    {
        if (ApplyDiseases(c))
            return;

        if (c.Aoe() && DiseasesUp(c))
        {
            if (TryCastRank(c.me, c.target, PESTILENCE)) return;
            if (TryCastRank(c.me, c.target, HOWLING_BLAST)) return;
            if (TryCastRank(c.me, c.target, BLOOD_BOIL)) return;
        }

        // Rafale hurlante gratuite sur proc Brouillard givrant
        if (c.me->HasAura(AURA_FREEZING_FOG) && TryCastRank(c.me, c.target, HOWLING_BLAST))
            return;

        if (!c.inMelee)
            return;

        // Oblitérer > Frappe de sang > Frappe de givre (Machine à tuer s'applique seule)
        if (TryCastRank(c.me, c.target, OBLITERATE)) return;
        if (TryCastRank(c.me, c.target, BLOOD_STRIKE)) return;
        TryCastRank(c.me, c.target, FROST_STRIKE);
    }

    void Unholy(Ctx& c)
    {
        if (c.Aoe() && TryCastAt(c.me, c.target, DEATH_AND_DECAY))
            return;

        if (ApplyDiseases(c))
            return;

        if (c.Aoe() && DiseasesUp(c))
        {
            if (TryCastRank(c.me, c.target, PESTILENCE)) return;
            if (TryCastRank(c.me, c.target, BLOOD_BOIL)) return;
        }

        if (!c.inMelee)
            return;

        // Frappe du fléau > Frappe de sang > Voile mortel
        if (TryCastRank(c.me, c.target, SCOURGE_STRIKE)) return;
        if (TryCastRank(c.me, c.target, BLOOD_STRIKE)) return;
        TryCastRank(c.me, c.target, DEATH_COIL);
    }
}

void DeathKnight(Ctx& c)
{
    if (!c.target)
        return;

    EngageMelee(c);

    switch (c.tree)
    {
        case 0:  Blood(c);  break;
        case 1:  Frost(c);  break;
        default: Unholy(c); break;
    }
}
} // namespace ModRotation
