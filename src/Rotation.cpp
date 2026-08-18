/*
 * mod-rotation — cœur du module : configuration, interception du sort
 * déclencheur, helpers communs et aiguillage classe/spécialisation.
 */

#include "Rotation.h"

#include "Chat.h"
#include "Config.h"
#include "CreatureAI.h"
#include "Duration.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"

namespace ModRotation
{
Settings config;

uint32 MaxRank(Player* p, uint32 firstRankId)
{
    uint32 best = 0;
    for (SpellInfo const* info = sSpellMgr->GetSpellInfo(firstRankId); info; info = info->GetNextRankSpell())
        if (p->HasSpell(info->Id))
            best = info->Id;

    return best;
}

static bool CanPayCost(Player* p, SpellInfo const* info)
{
    if (info->PowerType != POWER_MANA && info->PowerType != POWER_RAGE && info->PowerType != POWER_ENERGY)
        return true; // runes, etc. : vérifié par le cœur au moment du cast

    int32 cost = info->CalcPowerCost(p, info->GetSchoolMask());
    return int32(p->GetPower(Powers(info->PowerType))) >= cost;
}

static bool ReadyToCast(Player* p, SpellInfo const* info)
{
    if (!info)
        return false;

    if (p->HasSpellCooldown(info->Id))
        return false;

    if (p->GetGlobalCooldownMgr().HasGlobalCooldown(info))
        return false;

    return CanPayCost(p, info);
}

bool TryCast(Player* p, Unit* target, uint32 spellId)
{
    if (!spellId || !target)
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!ReadyToCast(p, info))
        return false;

    return p->CastSpell(target, spellId, false) == SPELL_CAST_OK;
}

bool TryCastRank(Player* p, Unit* target, uint32 firstRankId)
{
    return TryCast(p, target, MaxRank(p, firstRankId));
}

bool TryCastAt(Player* p, Unit* where, uint32 firstRankId)
{
    uint32 id = MaxRank(p, firstRankId);
    if (!id || !where)
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(id);
    if (!ReadyToCast(p, info))
        return false;

    return p->CastSpell(where->GetPositionX(), where->GetPositionY(), where->GetPositionZ(), id, false) == SPELL_CAST_OK;
}

bool KeepAuraOn(Player* p, Unit* target, uint32 firstRankId)
{
    if (!target || target->GetAuraOfRankedSpell(firstRankId, p->GetGUID()))
        return false;

    return TryCastRank(p, target, firstRankId);
}

bool KeepSelfBuff(Player* p, uint32 firstRankId)
{
    if (p->GetAuraOfRankedSpell(firstRankId))
        return false;

    return TryCastRank(p, p, firstRankId);
}

bool IsBleedImmune(Unit* target)
{
    uint32 type = target->GetCreatureType();
    return type == CREATURE_TYPE_MECHANICAL || type == CREATURE_TYPE_ELEMENTAL;
}

void PetAttack(Ctx& c)
{
    if (!c.target)
        return;

    if (Pet* pet = c.me->GetPet())
        if (pet->IsAlive() && !pet->GetVictim() && pet->AI())
            pet->AI()->AttackStart(c.target);
}

void EngageMelee(Ctx& c)
{
    if (c.target && c.me->GetVictim() != c.target)
        c.me->Attack(c.target, true);

    PetAttack(c);
}

void EngageRanged(Ctx& c)
{
    if (!c.target)
        return;

    PetAttack(c);

    // Tir automatique (75) pour les chasseurs
    if (c.me->getClass() == CLASS_HUNTER && !c.me->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
        c.me->CastSpell(c.target, 75, false);
}

static uint32 CountMeleeEnemies(Player* p, Unit* target)
{
    uint32 count = 0;
    for (Unit* attacker : p->getAttackers())
        if (attacker && attacker->IsAlive() && p->IsWithinMeleeRange(attacker))
            ++count;

    if (target && target->IsAlive() && p->IsWithinMeleeRange(target) && !p->getAttackers().count(target))
        ++count;

    return count;
}

static void ExecuteRotation(Player* p)
{
    if (!p->IsAlive())
        return;

    // Ne pas couper une incantation ou une canalisation en cours
    // (le Tir automatique n'est pas bloquant).
    if (p->IsNonMeleeSpellCast(false, false, true))
        return;

    Ctx c;
    c.me = p;

    Unit* sel = p->GetSelectedUnit();
    if (sel && sel->IsAlive())
    {
        if (p->IsValidAttackTarget(sel))
            c.target = sel;
        else if (sel != p && p->IsValidAssistTarget(sel))
            c.friendly = sel;
    }

    if (!c.friendly)
        c.friendly = p;

    if (c.target)
    {
        c.inMelee = p->IsWithinMeleeRange(c.target);
        c.enemies = CountMeleeEnemies(p, c.target);
    }

    c.tree = p->GetMostPointsTalentTree();

    switch (p->getClass())
    {
        case CLASS_WARRIOR:      Warrior(c);     break;
        case CLASS_PALADIN:      Paladin(c);     break;
        case CLASS_HUNTER:       Hunter(c);      break;
        case CLASS_ROGUE:        Rogue(c);       break;
        case CLASS_PRIEST:       Priest(c);      break;
        case CLASS_DEATH_KNIGHT: DeathKnight(c); break;
        case CLASS_SHAMAN:       Shaman(c);      break;
        case CLASS_MAGE:         Mage(c);        break;
        case CLASS_WARLOCK:      Warlock(c);     break;
        case CLASS_DRUID:        Druid(c);       break;
        default: break;
    }
}
} // namespace ModRotation

using namespace ModRotation;

class rotation_world_script : public WorldScript
{
public:
    rotation_world_script() : WorldScript("rotation_world_script") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        config.Enabled             = sConfigMgr->GetOption<bool>("Rotation.Enable", true);
        config.SpellId             = sConfigMgr->GetOption<uint32>("Rotation.SpellId", 47322);
        config.AutoLearn           = sConfigMgr->GetOption<bool>("Rotation.AutoLearn", true);
        config.Announce            = sConfigMgr->GetOption<bool>("Rotation.Announce", false);
        config.AnnounceMessage     = sConfigMgr->GetOption<std::string>("Rotation.Announce.Message",
            "One-button |cff4CFF00Rotation|r module active: place the Rotation spell from your spellbook on your action bar.");
        config.AoeThreshold        = sConfigMgr->GetOption<uint32>("Rotation.AoE.Threshold", 2);
        config.RageDumpThreshold   = sConfigMgr->GetOption<uint32>("Rotation.RageDump.Threshold", 50);
        config.EnergyDumpThreshold = sConfigMgr->GetOption<uint32>("Rotation.EnergyDump.Threshold", 60);
        config.HealInjuredPct      = sConfigMgr->GetOption<uint32>("Rotation.Heal.InjuredPct", 85);
        config.HealLowPct          = sConfigMgr->GetOption<uint32>("Rotation.Heal.LowPct", 60);
        config.HealUrgentPct       = sConfigMgr->GetOption<uint32>("Rotation.Heal.UrgentPct", 35);
    }
};

class rotation_player_script : public PlayerScript
{
public:
    rotation_player_script() : PlayerScript("rotation_player_script") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!config.Enabled)
            return;

        if (config.AutoLearn && !player->HasSpell(config.SpellId))
            player->learnSpell(config.SpellId);

        if (config.Announce && !config.AnnounceMessage.empty())
            ChatHandler(player->GetSession()).SendSysMessage(config.AnnounceMessage.c_str());
    }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!config.Enabled || !spell || spell->m_spellInfo->Id != config.SpellId)
            return;

        // Lancer un autre sort pendant la préparation du sort déclencheur n'est
        // pas sûr : on diffère la rotation d'un tick de mise à jour.
        ObjectGuid guid = player->GetGUID();
        player->m_Events.AddEventAtOffset([guid]()
        {
            if (Player* p = ObjectAccessor::FindPlayer(guid))
                ExecuteRotation(p);
        }, Milliseconds(1));
    }
};

void AddRotationScripts()
{
    new rotation_world_script();
    new rotation_player_script();
}
