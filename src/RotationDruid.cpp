/*
 * Druide — Equilibre (0), Farouche (1), Restauration (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 * Farouche : la rotation suit la forme actuelle (ours = tank, sinon félin).
 */

#include "Rotation.h"
#include "SpellAuras.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 MOONKIN_FORM   = 24858;
    constexpr uint32 CAT_FORM       = 768;

    constexpr uint32 FAERIE_FIRE    = 770;
    constexpr uint32 FAERIE_FIRE_FERAL = 16857;
    constexpr uint32 INSECT_SWARM   = 5570;
    constexpr uint32 MOONFIRE       = 8921;
    constexpr uint32 WRATH          = 5176;
    constexpr uint32 STARFIRE       = 2912;
    constexpr uint32 STARFALL       = 48505;
    constexpr uint32 HURRICANE      = 16914;

    constexpr uint32 MANGLE_CAT     = 33876;
    constexpr uint32 MANGLE_BEAR    = 33878;
    constexpr uint32 RAKE           = 1822;
    constexpr uint32 RIP            = 1079;
    constexpr uint32 SHRED          = 5221;
    constexpr uint32 CLAW           = 1082;
    constexpr uint32 SAVAGE_ROAR    = 52610;
    constexpr uint32 TIGERS_FURY    = 5217;
    constexpr uint32 LACERATE       = 33745;
    constexpr uint32 MAUL           = 6807;
    constexpr uint32 SWIPE_BEAR     = 779;

    constexpr uint32 REJUVENATION   = 774;
    constexpr uint32 REGROWTH       = 8936;
    constexpr uint32 LIFEBLOOM      = 33763;
    constexpr uint32 WILD_GROWTH    = 48438;
    constexpr uint32 NOURISH        = 50464;
    constexpr uint32 HEALING_TOUCH  = 5185;
    constexpr uint32 SWIFTMEND      = 18562;

    constexpr uint32 AURA_ECLIPSE_LUNAR = 48518;
    constexpr uint32 AURA_ECLIPSE_SOLAR = 48517;

    void Balance(Ctx& c)
    {
        // Forme de sélénien si connue
        if (c.me->HasSpell(MOONKIN_FORM) && c.me->GetShapeshiftForm() != FORM_MOONKIN
            && TryCast(c.me, c.me, MOONKIN_FORM))
            return;

        if (!c.target)
            return;

        // Lucioles > Essaim d'insectes > Feu lunaire entretenus
        if (KeepAuraOn(c.me, c.target, FAERIE_FIRE)) return;
        if (KeepAuraOn(c.me, c.target, INSECT_SWARM)) return;
        if (KeepAuraOn(c.me, c.target, MOONFIRE)) return;

        if (c.Aoe())
        {
            if (TryCast(c.me, c.me, MaxRank(c.me, STARFALL))) return;
            if (TryCastAt(c.me, c.target, HURRICANE)) return;
        }

        // Eclipse : lunaire → Feu stellaire, solaire → Colère
        if (c.me->HasAura(AURA_ECLIPSE_LUNAR) && TryCastRank(c.me, c.target, STARFIRE))
            return;
        if (c.me->HasAura(AURA_ECLIPSE_SOLAR) && TryCastRank(c.me, c.target, WRATH))
            return;

        // Remplisseur : Colère (repli Feu stellaire)
        if (TryCastRank(c.me, c.target, WRATH)) return;
        TryCastRank(c.me, c.target, STARFIRE);
    }

    void FeralBear(Ctx& c)
    {
        if (!c.inMelee)
            return;

        // Mutiler (ours) > Lucioles (farouche) > Lacérer x5 > Balayage (AoE) > Mutilation (dump)
        if (TryCastRank(c.me, c.target, MANGLE_BEAR)) return;
        if (TryCastRank(c.me, c.target, FAERIE_FIRE_FERAL)) return;

        Aura* lacerate = c.target->GetAuraOfRankedSpell(LACERATE, c.me->GetGUID());
        if ((!lacerate || lacerate->GetStackAmount() < 5) && TryCastRank(c.me, c.target, LACERATE))
            return;

        if (c.Aoe() && TryCastRank(c.me, c.target, SWIPE_BEAR))
            return;

        if (!c.me->GetCurrentSpell(CURRENT_MELEE_SPELL)
            && c.me->GetPower(POWER_RAGE) >= config.RageDumpThreshold * 10)
            TryCastRank(c.me, c.target, MAUL);
    }

    void FeralCat(Ctx& c)
    {
        // Forme de félin si besoin
        if (c.me->GetShapeshiftForm() != FORM_CAT)
        {
            TryCast(c.me, c.me, CAT_FORM);
            return;
        }

        if (!c.inMelee)
            return;

        uint8 cp = c.me->GetComboPoints(c.target);

        // Fureur du tigre quand l'énergie est basse
        if (c.me->GetPower(POWER_ENERGY) < 40 && TryCast(c.me, c.me, MaxRank(c.me, TIGERS_FURY)))
            return;

        // Rugissement sauvage dès 1 point de combo
        if (cp >= 1 && !c.me->HasAura(SAVAGE_ROAR) && c.me->HasSpell(SAVAGE_ROAR)
            && TryCast(c.me, c.target, SAVAGE_ROAR))
            return;

        // Déchirure à 5 points de combo
        if (cp >= 5 && !IsBleedImmune(c.target)
            && !c.target->GetAuraOfRankedSpell(RIP, c.me->GetGUID())
            && TryCastRank(c.me, c.target, RIP))
            return;

        // Griffure et Mutiler (félin) entretenus
        if (!IsBleedImmune(c.target) && KeepAuraOn(c.me, c.target, RAKE))
            return;
        if (KeepAuraOn(c.me, c.target, MANGLE_CAT))
            return;

        // Générateur : Lambeau (dans le dos), replis Mutiler puis Griffe
        if (TryCastRank(c.me, c.target, SHRED)) return;
        if (TryCastRank(c.me, c.target, MANGLE_CAT)) return;
        TryCastRank(c.me, c.target, CLAW);
    }

    void Feral(Ctx& c)
    {
        if (!c.target)
            return;

        ShapeshiftForm form = c.me->GetShapeshiftForm();
        if (form == FORM_BEAR || form == FORM_DIREBEAR)
            FeralBear(c);
        else
            FeralCat(c);
    }

    void Restoration(Ctx& c)
    {
        Unit* ally = c.friendly;
        float pct = ally->GetHealthPct();

        if (pct < config.HealUrgentPct)
        {
            // Prompte guérison consomme Récupération/Rétablissement
            if (TryCast(c.me, ally, SWIFTMEND)) return;
            if (KeepAuraOn(c.me, ally, REJUVENATION)) return;
            if (TryCastRank(c.me, ally, NOURISH)) return;
            TryCastRank(c.me, ally, HEALING_TOUCH);
            return;
        }
        if (pct < config.HealLowPct)
        {
            if (KeepAuraOn(c.me, ally, REJUVENATION)) return;
            if (KeepAuraOn(c.me, ally, REGROWTH)) return;
            if (TryCastRank(c.me, ally, WILD_GROWTH)) return;
            if (TryCastRank(c.me, ally, NOURISH)) return;
            TryCastRank(c.me, ally, HEALING_TOUCH);
            return;
        }
        if (pct < config.HealInjuredPct)
        {
            if (KeepAuraOn(c.me, ally, REJUVENATION)) return;
            if (ally != c.me && c.me->HasSpell(LIFEBLOOM) && KeepAuraOn(c.me, ally, LIFEBLOOM)) return;
            TryCastRank(c.me, ally, NOURISH);
            return;
        }

        // Personne à soigner : contribution légère en dégâts
        if (c.target)
        {
            if (KeepAuraOn(c.me, c.target, MOONFIRE)) return;
            TryCastRank(c.me, c.target, WRATH);
        }
    }
}

void Druid(Ctx& c)
{
    if (c.target && c.tree == 1)
        EngageMelee(c);

    switch (c.tree)
    {
        case 0:  Balance(c);     break;
        case 1:  Feral(c);       break;
        default: Restoration(c); break;
    }
}
} // namespace ModRotation
