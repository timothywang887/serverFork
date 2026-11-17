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
enum ROTATION{//To the left.
	ROTATION_0,
	ROTATION_90,
	ROTATION_180,
	ROTATION_270,
};
enum ROOM_TYPE{
	ROOM_1x1_INT_OCCUPIED = 0x0,
	ROOM_1x1_INT_AIR = 0x1,

	ROOM_1x1_HALLWAY,
	ROOM_1x1_STAIR_MIDDLE,
	ROOM_1x1_STAIR_UP,
	ROOM_1x1_STAIR_DOWN,
	ROOM_1x1_COURTYARD,

};
struct __attribute__((packed)) CastleData_ROOM{
	enum ROTATION rotation;
	enum ROOM_TYPE room_type; //TODO: this should map to a lookup table of room types, mainly to get a map of what grid places they take up
				  /* Every room must have the ability to connect on every edge horizontally. STAIR_UP must have a STAIR_MIDDLE or STAIR_DOWN above it; STAIR_DOWN must have a STAIR_MIDDLE or STAIR_UP below it; STAIR_MIDDLE must have a stair type both above and below it, and is not to be placed on the basement or top floors. COURTYARD can only be placed on the ground floor and must have a column of INT_AIR above it to the top. Every room on floors above ground must not have any INT_AIR below them until the basement (where INT_AIR is the ground).*/
	uint64_t doors; //Assume rotation of 0deg; doors is a bitmap starting at the rightmost bottommost face and going counterclockwise until it reaches that face again.
			/*Example: a L-shaped room
			 *  6
			 * 5X7
			 *  | 8 9
			 * 4X-X-X0
			 *  3 2 1
			 * Numbers are the #-th bit in the bitmap. If 0, no door; if 1 a door exists.
			 * When we place the map to verify it, we must verify the following, in this order:
			 * 	- No rooms overlap.
			 * 	- Every room is connected physically to every other room.
			 * 	- Every room has a door.
			 * 	- On levels other than ground level:
			 * 		- Every door is next to another door.
			 * 	- On ground level:
			 * 		- Every door not next to another door or a wall can pathfind to the outside of the castle, without changing floors.
			 * 		- Every other door is next to another door and not a wall or air.
			 * 	- Every room's origin cube can pathfind to one of the outside doors.
			 * */
};
struct __attribute__((packed)) PositionOrientationVelocity{
	double x;
	double y;
	double z;

	double yaw;
	double pitch;
	
	double vx;
	double vy;
	double vz;
};
struct __attribute__((packed)) CastleData_CASTLE{
	struct CastleData_ROOM rooms[5][5][4];
	uint8_t banner; //Unused. Do this later.
	uint8_t seed[9]; //Seed for worldgen
	uint8_t difficulties; //bitmap of difficulties
	struct PositionOrientationVelocity spawnpos;
	//I have heard that enemy data is actually on the client.
};

#endif
