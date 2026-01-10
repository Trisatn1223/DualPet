class AnimalCompanionPlayerScript : public PlayerScript
{
public:
    AnimalCompanionPlayerScript() : PlayerScript("AnimalCompanionPlayerScript") { }

    void OnLogin(Player* player) override
    {
        TrySummon(player);
    }

    void OnLogout(Player* player) override
    {
        Despawn(player);
    }

    void OnUpdate(Player* player, uint32 /*diff*/) override
    {
        if (!player->IsInWorld())
            return;

        // Wenn Hauptpet existiert aber kein Companion → neu beschwören
        if (player->GetPet() && !HasCompanion(player))
            TrySummon(player);

        // Wenn Hauptpet weg → Companion entfernen
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
        if (player->IsBot())
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
        companion->SetCanModifyStats(true);

        companion->InitStatsForLevel(player->getLevel());
        companion->InitPetCreateSpells();

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
