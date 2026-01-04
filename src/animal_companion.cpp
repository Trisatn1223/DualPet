#include "ScriptMgr.h"
#include "Player.h"
#include "Pet.h"
#include "DatabaseEnv.h"

#define COMPANION_DAMAGE_MOD 0.5f

class AnimalCompanionPlayerScript : public PlayerScript
{
public:
    AnimalCompanionPlayerScript() : PlayerScript("AnimalCompanionPlayerScript") { }

    void OnLogin(Player* player) override
    {
        SummonCompanion(player);
    }

    void OnLogout(Player* player) override
    {
        DespawnCompanion(player);
    }

    void OnPetSummon(Player* player, Pet* /*pet*/) override
    {
        SummonCompanion(player);
    }

    void OnPetRemove(Player* player, Pet* /*pet*/) override
    {
        DespawnCompanion(player);
    }

private:
    ObjectGuid companionGuid;

    void SummonCompanion(Player* player)
    {
        if (player->getClass() != CLASS_HUNTER)
            return;

        if (!player->GetPet())
            return;

        if (player->GetGuardianPet())
            return;

        QueryResult result = CharacterDatabase.PQuery(
            "SELECT entry FROM character_pet WHERE owner = %u AND slot = 0",
            player->GetGUID().GetCounter());

        if (!result)
            return;

        uint32 entry = result->Fetch()[0].Get<uint32>();

        Pet* companion = new Pet(player, HUNTER_PET);
        if (!companion->CreateBaseAtCreatureEntry(entry, player))
        {
            delete companion;
            return;
        }

        companion->SetReactState(REACT_ASSIST);
        companion->SetCanModifyStats(false);
        companion->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);

        companion->Summon();
        companion->SetOwnerGUID(player->GetGUID());

        companion->SetMaxHealth(companion->GetMaxHealth() * COMPANION_DAMAGE_MOD);
        companion->SetHealth(companion->GetMaxHealth());
        companion->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE,
            companion->GetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE) * COMPANION_DAMAGE_MOD);
        companion->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE,
            companion->GetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE) * COMPANION_DAMAGE_MOD);

        companionGuid = companion->GetGUID();
    }

    void DespawnCompanion(Player* player)
    {
        if (!companionGuid)
            return;

        if (Creature* pet = ObjectAccessor::GetCreature(*player, companionGuid))
            pet->DespawnOrUnsummon();

        companionGuid.Clear();
    }
};

void AddAnimalCompanionScripts()
{
    new AnimalCompanionPlayerScript();
}
