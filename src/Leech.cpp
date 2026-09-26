/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license
 */

#include "Leech.h"
#include <cmath>

// ---------------------------------------------------------------------------
// Cached configuration (filled in OnAfterConfigLoad)
// ---------------------------------------------------------------------------
class Leech_WorldScript : public WorldScript
{
public:
    Leech_WorldScript() : WorldScript("Leech_WorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        _enabled         = sConfigMgr->GetOption<bool>("Leech.Enable", false);
        _dungeonsOnly    = sConfigMgr->GetOption<bool>("Leech.DungeonsOnly", true);
        _amount          = sConfigMgr->GetOption<float>("Leech.Amount", 0.05f);
        _allowSelfDamage = sConfigMgr->GetOption<bool>("Leech.AllowSelfDamage", false);
        _requiredItem    = sConfigMgr->GetOption<uint32>("Leech.RequiredItemId", 0);

        std::string petMode = sConfigMgr->GetOption<std::string>("Leech.PetDamage", "pet");
        if (petMode == "none")
            _petMode = LEECH_PET_NONE;
        else if (petMode == "owner")
            _petMode = LEECH_PET_OWNER;
        else
            _petMode = LEECH_PET_PET;
    }

    static bool    Enabled()         { return _enabled; }
    static bool    DungeonsOnly()    { return _dungeonsOnly; }
    static float   Amount()          { return _amount; }
    static bool    AllowSelfDamage() { return _allowSelfDamage; }
    static uint32  RequiredItem()    { return _requiredItem; }
    static LeechPetMode PetMode()    { return _petMode; }

private:
    static bool    _enabled;
    static bool    _dungeonsOnly;
    static float   _amount;
    static bool    _allowSelfDamage;
    static uint32  _requiredItem;
    static LeechPetMode _petMode;
};

bool    Leech_WorldScript::_enabled         = false;
bool    Leech_WorldScript::_dungeonsOnly    = true;
float   Leech_WorldScript::_amount          = 0.05f;
bool    Leech_WorldScript::_allowSelfDamage = false;
uint32  Leech_WorldScript::_requiredItem    = 0;
LeechPetMode Leech_WorldScript::_petMode    = LEECH_PET_PET;

// ---------------------------------------------------------------------------
// Unit script (leech on damage)
// ---------------------------------------------------------------------------
class Leech_UnitScript : public UnitScript
{
public:
    Leech_UnitScript() : UnitScript("Leech_UnitScript") { }

    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
    {
        // 1. Cheap mandatory checks
        if (!attacker || !Leech_WorldScript::Enabled())
            return;

        // 2. Self-damage filter (reflect, fall damage, etc.)
        if (!Leech_WorldScript::AllowSelfDamage() && attacker == victim)
            return;

        // 3. Attacker must be player or player's pet
        bool isPet = attacker->GetOwner() && attacker->GetOwner()->GetTypeId() == TYPEID_PLAYER;
        if (!isPet && attacker->GetTypeId() != TYPEID_PLAYER)
            return;

        Player* player = isPet ? attacker->GetOwner()->ToPlayer() : attacker->ToPlayer();
        if (!player)
            return;

        // 4. Dungeon-only filter
        if (Leech_WorldScript::DungeonsOnly() && !player->GetMap()->IsDungeon())
            return;

        // 5. Required item filter
        uint32 requiredItem = Leech_WorldScript::RequiredItem();
        if (requiredItem != 0 && !player->HasItemCount(requiredItem, 1, false))
            return;

        // 6. Apply heal
        float amount = Leech_WorldScript::Amount();
        int32 bp1 = static_cast<int32>(std::lround(amount * float(damage)));

        Unit* caster;
        Unit* target;

        if (isPet)
        {
            switch (Leech_WorldScript::PetMode())
            {
                case LEECH_PET_NONE:  return;
                case LEECH_PET_OWNER: caster = target = player;    break;
                default:              caster = target = attacker;  break; // LEECH_PET_PET
            }
        }
        else
        {
            caster = target = player;
        }

        caster->CastCustomSpell(target, SPELL_HEAL, &bp1, nullptr, nullptr, true);
    }
};

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void AddSC_mod_leech()
{
    new Leech_WorldScript();
    new Leech_UnitScript();
}
