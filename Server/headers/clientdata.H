#ifndef H_CLIENTDATA
#define H_CLIENTDATA
#include <stdint.h>
struct __attribute__((packed)) Item_SkillTree{
	uint8_t unlocked[16];
};
enum ITEM_WEAPONS {
        ITEM_WEAPON_CHEESE_SANDWICH, //Temporary, an example. Client, once again, deals with it.
};
struct __attribute__((packed)) Item_WEAPON{
        enum ITEM_WEAPONS type;
        struct Item_SkillTree tree;
        uint8_t level;
        double xp;
};
enum ITEM_RELICS{
        ITEM_RELIC_CHEESE_SANDWICH, //Temporary, an example. Client deals with it.
};
struct __attribute__((packed)) GameSessionData_Player_Inventory{ //Temporary to the session
        struct Item_WEAPON weapon;
        enum ITEM_RELICS relic;
        //Also three consumables in the loadout which appear somewhere in the map
};

#endif
