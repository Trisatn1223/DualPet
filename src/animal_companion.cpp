#include "ScriptMgr.h"
#include "Player.h"
#include "Pet.h"
#include "ObjectAccessor.h"
#include "DatabaseEnv.h"

class AnimalCompanionPlayerScript : public PlayerScript
{
public:
    AnimalCompanionPlayerScript() : PlayerScript("AnimalCompanionPlayerScript") { }

    void OnLogin(Player* player)
    {
        TrySummon(player);
    }

    void OnLogout(Player* player)
    {
        Despawn(player);
    }

    void OnUpdate(Player* player, uint32 /*diff*/)
    {
        if (!player->IsInWorld())
            return;

        if (player->GetPet() && !HasCompanion(player))
            TrySummon(player);

        if (!player->GetPet() && HasCompanion(player))
            Despawn(player);
    }

private:
    ObjectGuid companionGuid;

    bool HasCompanion(Player* player)
    {
        return companionGuid && ObjectAccessor::GetCreature(*player, companionGuid);
    }

    void TrySummon(Player* player)
    {
        if (!player->GetSession())
            return;

        if (player->getClass() != CLASS_HUNTER)
            return;

        if (!player->GetPet())
            return;

        if (HasCompanion(player))
            return;

        SummonCompanion(player);
    }

 void SummonCompanion(Player* player)
{
    QueryResult result = CharacterDatabase.Query(
        "SELECT entry FROM character_pet WHERE owner = {} AND slot = 0",
        player->GetGUID().GetCounter());

    if (!result)
        return;

    uint32 entry = result->Fetch()[0].Get<uint32>();

    Pet* companion = new Pet(player, HUNTER_PET);

    Map* map = player->GetMap();
    if (!map)
    {
        delete companion;
        return;
    }

    uint32 phaseMask = player->GetPhaseMask();
    ObjectGuid::LowType guidlow = sObjectMgr->GeneratePetLowGuid(); // <--- hier geändert
    uint32 pet_slot = 0;

    if (!companion->Create(guidlow, map, phaseMask, entry, pet_slot))
    {
        delete companion;
        return;
    }

    companion->InitStatsForLevel(player->getLevel());
    companion->InitPetCreateSpells();

    companion->SetReactState(REACT_ASSIST);
    companion->SetCanModifyStats(true);

    companion->Summon();
    companion->AIM_Initialize();

    companionGuid = companion->GetGUID();
}



    void Despawn(Player* player)
    {
        if (!companionGuid)
            return;

        if (Creature* c = ObjectAccessor::GetCreature(*player, companionGuid))
            c->DespawnOrUnsummon();

        companionGuid.Clear();
    }
};

void AddAnimalCompanionScripts()
{
    new AnimalCompanionPlayerScript();
}
