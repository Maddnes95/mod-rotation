/*
 * mod-rotation — Rotation « un bouton » pour AzerothCore (branche Playerbot).
 * Déclarations communes : configuration, contexte, helpers et rotations.
 */

#ifndef MOD_ROTATION_H
#define MOD_ROTATION_H

#include "Player.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

namespace ModRotation
{
    struct Settings
    {
        bool   Enabled           = true;
        uint32 SpellId           = 47322;
        bool   AutoLearn         = true;
        bool   Announce          = true;
        uint32 AoeThreshold      = 2;
        uint32 RageDumpThreshold = 50;
        uint32 EnergyDumpThreshold = 60;
        uint32 HealInjuredPct    = 85;
        uint32 HealLowPct        = 60;
        uint32 HealUrgentPct     = 35;
    };

    extern Settings config;

    // Contexte d'une exécution de rotation (un appui sur le bouton).
    struct Ctx
    {
        Player* me       = nullptr;
        Unit*   target   = nullptr; // cible hostile valide (peut être nullptr)
        Unit*   friendly = nullptr; // cible amicale sélectionnée, sinon soi-même
        bool    inMelee  = false;
        uint32  enemies  = 0;       // ennemis au corps à corps
        uint8   tree     = 0;       // arbre de talents dominant (0/1/2)

        bool Aoe() const { return enemies >= config.AoeThreshold; }
    };

    // Helpers communs (définis dans Rotation.cpp)
    uint32 MaxRank(Player* p, uint32 firstRankId);
    bool   TryCast(Player* p, Unit* target, uint32 spellId);
    bool   TryCastRank(Player* p, Unit* target, uint32 firstRankId);
    bool   TryCastAt(Player* p, Unit* where, uint32 firstRankId);   // sort ciblé au sol
    bool   KeepAuraOn(Player* p, Unit* target, uint32 firstRankId); // (re)applique notre aura si absente
    bool   KeepSelfBuff(Player* p, uint32 firstRankId);             // buff personnel si absent
    bool   IsBleedImmune(Unit* target);
    void   EngageMelee(Ctx& c);
    void   EngageRanged(Ctx& c);
    void   PetAttack(Ctx& c);

    // Rotations par classe (un fichier chacune)
    void Warrior(Ctx& c);
    void Paladin(Ctx& c);
    void Hunter(Ctx& c);
    void Rogue(Ctx& c);
    void Priest(Ctx& c);
    void DeathKnight(Ctx& c);
    void Shaman(Ctx& c);
    void Mage(Ctx& c);
    void Warlock(Ctx& c);
    void Druid(Ctx& c);
}

#endif // MOD_ROTATION_H
