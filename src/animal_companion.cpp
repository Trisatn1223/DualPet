#include "ScriptMgr.h"
#include "Player.h"
#include "Pet.h"
#include "ObjectAccessor.h"
#include "DatabaseEnv.h"
#include "Config.h"

/*
 DualPet Module
 - Summons second pet from slot 0
 - Uses existing character_pet.id as GUID
 - Configurable: Enabled & DamageMultiplier
*/

class AnimalCompanionPlayerScript : public PlayerScript
{
public:
    AnimalCompanionPlayerScript()
        : PlayerScript("AnimalCompanionPlayerScript")
    {
        moduleEnabled = sConfigMgr->GetOption<bool>("DualPet.Enabled", true);
        damageMultiplier = sConfigMgr->GetOption<float>("DualPet.DamageMultiplier", 1.0f);
    }

    void OnLogin(Player* player)            // override entfernt
    {
        TrySummon(player);
    }

    void OnLogout(Player* player)           // override entfernt
    {
        Despawn(player);
    }

    void OnUpdate(Player* player, uint32 /*diff*/)  // override entfernt
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
    bool moduleEnabled;
    float damageMultiplier;

    bool HasCompanion(Player* player)
    {
        return companionGuid && ObjectAccessor::GetCreature(*player, companionGuid);
    }

    void TrySummon(Player* player)
    {
        if (!moduleEnabled)             // Modul deaktiviert
            return;

        if (!player->GetSession())      // Playerbots ausschließen
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
        // DB-Eintrag des Stall-Pets lesen
        QueryResult result = CharacterDatabase.Query(
            "SELECT id, entry FROM character_pet WHERE owner = {} AND slot = 0",
            player->GetGUID().GetCounter());

        if (!result)
            return;

        Field* fields = result->Fetch();
        uint32 petDBId = fields[0].Get<uint32>();
        uint32 entry   = fields[1].Get<uint32>();

        // GUID vom DB-Pet verwenden
        ObjectGuid petGuid = MAKE_NEW_GUID(petDBId, 0, HIGHGUID_PET);

        // Pet erstellen
        Pet* companion = new Pet(player, HUNTER_PET);
        if (!companion->Create(petGuid.GetCounter(), player->GetMap(), player->GetPhaseMask(), entry, 0))
        {
            delete companion;
            return;
        }

        // Stats & Spells initialisieren
        companion->InitStatsForLevel(player->getLevel());
        companion->InitPetCreateSpells();

        // Damage multiplier anwenden
        for (auto &spell : companion->GetPetSpells())
        {
            if (spell)
                spell->SetDamage(spell->GetDamage() * damageMultiplier);
        }

        // Verhalten & Summon
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
