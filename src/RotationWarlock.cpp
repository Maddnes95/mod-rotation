/*
 * Démoniste — Affliction (0), Démonologie (1), Destruction (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 */

#include "Rotation.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 LIFE_TAP       = 1454;
    constexpr uint32 CORRUPTION     = 172;
    constexpr uint32 CURSE_OF_AGONY = 980;
    constexpr uint32 CURSE_OF_DOOM  = 603;
    constexpr uint32 UNSTABLE_AFFLICTION = 30108;
    constexpr uint32 HAUNT          = 48181;
    constexpr uint32 IMMOLATE       = 348;
    constexpr uint32 CONFLAGRATE    = 17962;
    constexpr uint32 CHAOS_BOLT     = 50796;
    constexpr uint32 INCINERATE     = 29722;
    constexpr uint32 SHADOW_BOLT    = 686;
    constexpr uint32 SOUL_FIRE      = 6353;
    constexpr uint32 DRAIN_SOUL     = 1120;
    constexpr uint32 SEED_OF_CORRUPTION = 27243;

    // Coeur de la fournaise (Molten Core) et Décimation : tous les rangs du buff
    constexpr uint32 AURA_MOLTEN_CORE[] = { 47383, 71162, 71165 };
    constexpr uint32 AURA_DECIMATION[]  = { 63165, 63167 };

    template <size_t N>
    bool HasAnyAura(Player* p, uint32 const (&ids)[N])
    {
        for (uint32 id : ids)
            if (p->HasAura(id))
                return true;

        return false;
    }

    // Connexion : regagne de la mana sur la vie quand c'est nécessaire
    bool NeedLifeTap(Ctx& c)
    {
        if (c.me->GetPowerPct(POWER_MANA) > 20.0f || c.me->GetHealthPct() < 40.0f)
            return false;

        return TryCastRank(c.me, c.me, LIFE_TAP);
    }

    void Affliction(Ctx& c)
    {
        if (NeedLifeTap(c))
            return;

        if (c.Aoe() && TryCastRank(c.me, c.target, SEED_OF_CORRUPTION))
            return;

        // Hantise > Corruption > Affliction instable > Malédiction d'agonie
        if (TryCastRank(c.me, c.target, HAUNT)) return;
        if (KeepAuraOn(c.me, c.target, CORRUPTION)) return;
        if (KeepAuraOn(c.me, c.target, UNSTABLE_AFFLICTION)) return;
        if (KeepAuraOn(c.me, c.target, CURSE_OF_AGONY)) return;

        // Drain d'âme sous 25 %, sinon Trait de l'ombre
        if (c.target->HealthBelowPct(25) && TryCastRank(c.me, c.target, DRAIN_SOUL))
            return;

        TryCastRank(c.me, c.target, SHADOW_BOLT);
    }

    void Demonology(Ctx& c)
    {
        if (NeedLifeTap(c))
            return;

        if (c.Aoe() && TryCastRank(c.me, c.target, SEED_OF_CORRUPTION))
            return;

        // Malédiction d'apocalypse (repli agonie) > Corruption > Immolation
        if (MaxRank(c.me, CURSE_OF_DOOM))
        {
            if (KeepAuraOn(c.me, c.target, CURSE_OF_DOOM))
                return;
        }
        else if (KeepAuraOn(c.me, c.target, CURSE_OF_AGONY))
            return;
        if (KeepAuraOn(c.me, c.target, CORRUPTION)) return;
        if (KeepAuraOn(c.me, c.target, IMMOLATE)) return;

        // Feu de l'âme avec Décimation, Incinérer avec Coeur de la fournaise
        if (HasAnyAura(c.me, AURA_DECIMATION) && TryCastRank(c.me, c.target, SOUL_FIRE))
            return;
        if (HasAnyAura(c.me, AURA_MOLTEN_CORE) && TryCastRank(c.me, c.target, INCINERATE))
            return;

        TryCastRank(c.me, c.target, SHADOW_BOLT);
    }

    void Destruction(Ctx& c)
    {
        if (NeedLifeTap(c))
            return;

        if (c.Aoe() && TryCastRank(c.me, c.target, SEED_OF_CORRUPTION))
            return;

        // Immolation entretenue > Conflagration > Trait du chaos > Malédiction d'apocalypse
        if (KeepAuraOn(c.me, c.target, IMMOLATE)) return;
        if (TryCastRank(c.me, c.target, CONFLAGRATE)) return;
        if (TryCastRank(c.me, c.target, CHAOS_BOLT)) return;
        if (KeepAuraOn(c.me, c.target, CURSE_OF_DOOM)) return;

        // Incinérer en remplisseur (repli Trait de l'ombre à bas niveau)
        if (TryCastRank(c.me, c.target, INCINERATE)) return;
        TryCastRank(c.me, c.target, SHADOW_BOLT);
    }
}

void Warlock(Ctx& c)
{
    if (!c.target)
        return;

    PetAttack(c);

    switch (c.tree)
    {
        case 0:  Affliction(c);  break;
        case 1:  Demonology(c);  break;
        default: Destruction(c); break;
    }
}
} // namespace ModRotation
