/*
 * Prêtre — Discipline (0), Sacré (1), Ombre (2).
 * Priorités d'après les guides Wowhead WotLK Classic.
 */

#include "Rotation.h"

namespace ModRotation
{
namespace
{
    constexpr uint32 SHADOWFORM       = 15473;
    constexpr uint32 VAMPIRIC_TOUCH   = 34914;
    constexpr uint32 DEVOURING_PLAGUE = 2944;
    constexpr uint32 SW_PAIN          = 589;
    constexpr uint32 MIND_BLAST       = 8092;
    constexpr uint32 SW_DEATH         = 32379;
    constexpr uint32 MIND_FLAY        = 15407;
    constexpr uint32 MIND_SEAR        = 48045;

    constexpr uint32 PW_SHIELD        = 17;
    constexpr uint32 WEAKENED_SOUL    = 6788;
    constexpr uint32 PENANCE          = 47540;
    constexpr uint32 PRAYER_OF_MENDING = 33076;
    constexpr uint32 RENEW            = 139;
    constexpr uint32 FLASH_HEAL       = 2061;
    constexpr uint32 GREATER_HEAL     = 2060;
    constexpr uint32 CIRCLE_OF_HEALING = 34861;

    constexpr uint32 SMITE            = 585;
    constexpr uint32 HOLY_FIRE        = 14914;

    void Shadow(Ctx& c)
    {
        // Forme d'ombre si connue
        if (c.me->HasSpell(SHADOWFORM) && !c.me->HasAura(SHADOWFORM) && TryCast(c.me, c.me, SHADOWFORM))
            return;

        if (!c.target)
            return;

        // AoE : Fouet mental (canalisé sur la cible)
        if (c.Aoe() && TryCastRank(c.me, c.target, MIND_SEAR))
            return;

        // Entretien des DoTs : Toucher vampirique > Peste dévorante > Mot de l'ombre : Douleur
        if (KeepAuraOn(c.me, c.target, VAMPIRIC_TOUCH)) return;
        if (KeepAuraOn(c.me, c.target, DEVOURING_PLAGUE)) return;
        if (KeepAuraOn(c.me, c.target, SW_PAIN)) return;

        // Attaque mentale, Mot de l'ombre : Mort, puis Fouet mental en remplisseur
        if (TryCastRank(c.me, c.target, MIND_BLAST)) return;
        if (TryCastRank(c.me, c.target, SW_DEATH)) return;
        TryCastRank(c.me, c.target, MIND_FLAY);
    }

    // Mode dégâts léger pour Discipline/Sacré quand personne n'est blessé
    void HolyDamage(Ctx& c)
    {
        if (!c.target)
            return;

        if (KeepAuraOn(c.me, c.target, SW_PAIN)) return;
        if (TryCastRank(c.me, c.target, HOLY_FIRE)) return;
        TryCastRank(c.me, c.target, SMITE);
    }

    void Discipline(Ctx& c)
    {
        Unit* ally = c.friendly;
        float pct = ally->GetHealthPct();

        // Mot de pouvoir : Bouclier si possible (pas d'Âme affaiblie)
        if (pct < config.HealInjuredPct
            && !ally->GetAuraOfRankedSpell(PW_SHIELD)
            && !ally->HasAura(WEAKENED_SOUL)
            && TryCastRank(c.me, ally, PW_SHIELD))
            return;

        if (pct < config.HealUrgentPct)
        {
            if (TryCastRank(c.me, ally, PENANCE)) return;
            TryCastRank(c.me, ally, FLASH_HEAL);
            return;
        }
        if (pct < config.HealLowPct)
        {
            if (TryCastRank(c.me, ally, PENANCE)) return;
            if (TryCastRank(c.me, ally, PRAYER_OF_MENDING)) return;
            TryCastRank(c.me, ally, FLASH_HEAL);
            return;
        }
        if (pct < config.HealInjuredPct)
        {
            if (TryCastRank(c.me, ally, PRAYER_OF_MENDING)) return;
            if (KeepAuraOn(c.me, ally, RENEW)) return;
            TryCastRank(c.me, ally, FLASH_HEAL);
            return;
        }

        HolyDamage(c);
    }

    void Holy(Ctx& c)
    {
        Unit* ally = c.friendly;
        float pct = ally->GetHealthPct();

        if (pct < config.HealUrgentPct)
        {
            if (TryCastRank(c.me, ally, FLASH_HEAL)) return;
            TryCastRank(c.me, ally, GREATER_HEAL);
            return;
        }
        if (pct < config.HealLowPct)
        {
            if (TryCastRank(c.me, ally, CIRCLE_OF_HEALING)) return;
            if (TryCastRank(c.me, ally, PRAYER_OF_MENDING)) return;
            TryCastRank(c.me, ally, GREATER_HEAL);
            return;
        }
        if (pct < config.HealInjuredPct)
        {
            if (TryCastRank(c.me, ally, PRAYER_OF_MENDING)) return;
            if (KeepAuraOn(c.me, ally, RENEW)) return;
            TryCastRank(c.me, ally, FLASH_HEAL);
            return;
        }

        HolyDamage(c);
    }
}

void Priest(Ctx& c)
{
    switch (c.tree)
    {
        case 0:  Discipline(c); break;
        case 1:  Holy(c);       break;
        default: Shadow(c);     break;
    }
}
} // namespace ModRotation
