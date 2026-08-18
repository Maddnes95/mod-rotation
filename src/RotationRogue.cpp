/*
 * Voleur — Assassinat (0), Combat (1), Finesse (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 */

#include "Rotation.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 SINISTER_STRIKE = 1752;
    constexpr uint32 HEMORRHAGE      = 16511;
    constexpr uint32 MUTILATE        = 1329;
    constexpr uint32 SLICE_AND_DICE  = 5171;
    constexpr uint32 RUPTURE         = 1943;
    constexpr uint32 ENVENOM         = 32645;
    constexpr uint32 EVISCERATE      = 2098;
    constexpr uint32 HUNGER_FOR_BLOOD = 51662;
    constexpr uint32 FAN_OF_KNIVES   = 51723;

    // Estocade si absente (n'importe quel nombre de points)
    bool KeepSliceAndDice(Ctx& c, uint8 cp)
    {
        if (cp == 0 || c.me->GetAuraOfRankedSpell(SLICE_AND_DICE))
            return false;

        return TryCastRank(c.me, c.me, SLICE_AND_DICE);
    }

    void Assassination(Ctx& c, uint8 cp)
    {
        if (KeepSliceAndDice(c, cp))
            return;

        // Soif de sang : nécessite un saignement sur la cible
        if (c.me->HasSpell(HUNGER_FOR_BLOOD) && !c.me->HasAura(HUNGER_FOR_BLOOD))
        {
            if (c.target->GetAuraOfRankedSpell(RUPTURE, c.me->GetGUID()))
            {
                if (TryCast(c.me, c.me, HUNGER_FOR_BLOOD))
                    return;
            }
            else if (cp >= 1 && !IsBleedImmune(c.target) && TryCastRank(c.me, c.target, RUPTURE))
                return;
        }

        // Finition à 4-5 points : Envenimer (repli Eviscération sans poison)
        if (cp >= 4)
        {
            if (TryCastRank(c.me, c.target, ENVENOM)) return;
            if (TryCastRank(c.me, c.target, EVISCERATE)) return;
        }

        // Générateur : Mutilation (repli Attaque sournoise sans deux dagues)
        if (TryCastRank(c.me, c.target, MUTILATE)) return;
        TryCastRank(c.me, c.target, SINISTER_STRIKE);
    }

    void Combat(Ctx& c, uint8 cp)
    {
        if (KeepSliceAndDice(c, cp))
            return;

        if (cp >= 4 && !IsBleedImmune(c.target)
            && !c.target->GetAuraOfRankedSpell(RUPTURE, c.me->GetGUID())
            && TryCastRank(c.me, c.target, RUPTURE))
            return;

        if (cp >= 5 && TryCastRank(c.me, c.target, EVISCERATE))
            return;

        TryCastRank(c.me, c.target, SINISTER_STRIKE);
    }

    void Subtlety(Ctx& c, uint8 cp)
    {
        if (KeepSliceAndDice(c, cp))
            return;

        if (cp >= 4 && !IsBleedImmune(c.target)
            && !c.target->GetAuraOfRankedSpell(RUPTURE, c.me->GetGUID())
            && TryCastRank(c.me, c.target, RUPTURE))
            return;

        if (cp >= 5 && TryCastRank(c.me, c.target, EVISCERATE))
            return;

        // Générateur : Hémorragie (repli Attaque sournoise)
        if (TryCastRank(c.me, c.target, HEMORRHAGE)) return;
        TryCastRank(c.me, c.target, SINISTER_STRIKE);
    }
}

void Rogue(Ctx& c)
{
    if (!c.target)
        return;

    EngageMelee(c);

    // AoE : Eventail de couteaux
    if (c.Aoe() && c.inMelee && TryCast(c.me, c.me, MaxRank(c.me, FAN_OF_KNIVES)))
        return;

    if (!c.inMelee)
        return;

    uint8 cp = c.me->GetComboPoints(c.target);

    switch (c.tree)
    {
        case 0:  Assassination(c, cp); break;
        case 1:  Combat(c, cp);        break;
        default: Subtlety(c, cp);      break;
    }
}
} // namespace ModRotation
