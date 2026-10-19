#include "headers/protocol.H"
#include "headers/comm.H"
#include <cassert>
#include <stdio.h>
bool verifyToken(ENCVAL_TEMP token, UserSession * ses_assoc){ //TODO
	return true;
}
#ifdef WIP_AUTH
char * unencryptSPK(const char * const data, uint32_t data_length){ //TODO
	char * d2 = (char*)malloc(data_length);
	memcpy(d2, data, data_length);
	return d2;
}
char * encryptSymmetric(const char * const data, uint32_t data_length, ENCVAL_SYMMETRIC_TEMP key, uint32_t * lenres){ //TODO
	char * d2 =(char*) malloc(data_length);
	memcpy(d2, data, data_length);
	*lenres = data_length;
	return d2;
}

struct __attribute__((packed)) Protocol_Msg_AuthRQ{
	C_SEMIPERMA spt;
	ENCVAL_SYMMETRIC_TEMP sk;
};
struct __attribute__((packed)) Protocol_Msg_LoginRQ{
	char username[128];
	char password[128];
	ENCVAL_SYMMETRIC_TEMP sk;
};
struct __attribute__((packed)) Protocol_Msg_SC_TOKENRSP{
	C_SESSIONTOK st;
};
struct __attribute__((packed)) Protocol_Msg_SC_TOKENRSPWSP{
	C_SEMIPERMA spt;
	C_SESSIONTOK st;
};
#endif
enum MAP_DIFFICULTY{
	DIFFICULTY_TRIVIAL = 0x1,
	DIFFICULTY_EASY = 0x2,
	DIFFICULTY_MEDIUM = 0x4,
	DIFFICULTY_HARD = 0x8,
	DIFFICULTY_STONED = 0x10
};
enum MAP_PUBLICITY{
	PUBLICITY_PRIVATE, //Only people with join code.
	PUBLICITY_FRIENDS, //Only friends.
	PUBLICITY_PUBLIC   //Everyone.
};
struct __attribute__((packed)) Protocol_Msg_CS_MAPJOINRQ{
	uint64_t map_id;
	enum MAP_PUBLICITY publicity;
	enum MAP_DIFFICULTY difficulty_requested;
	uint8_t max_players;
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
struct __attribute__((packed)) CastleData_CASTLE{
	struct CastleData_ROOM rooms[5][5][4];
	uint8_t banner; //Unused. Do this later.
	uint8_t seed[9]; //Seed for worldgen
	uint8_t difficulties; //bitmap of difficulties
	//I have heard that enemy data is actually on the client.
};
struct __attribute__((packed)) Item_SkillTree{
	uint8_t tree; //Unused. Do this later.
};
enum ITEM_WEAPONS {
	ITEM_WEAPON_CHEESE_SANDWICH, //Temporary, an example. Client, once again, deals with it.
};
struct __attribute__((packed)) Item_WEAPON{
	enum ITEM_WEAPONS type;
	struct Item_SkillTree tree;
};
enum ITEM_RELICS{
	ITEM_RELIC_CHEESE_SANDWICH, //Temporary, an example. Client deals with it.
};
struct __attribute__((packed)) GameSessionData_Player_Inventory{ //Temporary to the session
	struct Item_WEAPON weapon;
	enum ITEM_RELICS relic;
	//Also three consumables in the loadout which appear somewhere in the map
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
struct __attribute__((packed)) GameSessionData_PLAYER_INIT{
	struct PositionOrientationVelocity pos;
	struct GameSessionData_Player_Inventory inv;
	uint8_t player_no;
};
struct __attribute__((packed)) Protocol_Msg_SC_MAPJOINRSP{
	//Castle data
	struct CastleData_CASTLE castle;
	//Map data
	enum MAP_DIFFICULTY difficulty; //Differs from difficulty_requested if the map doesn't allow it.
	//Session join code
	uint64_t join_code;
	//Player position/inventory/etc data
	struct GameSessionData_PLAYER_INIT you; //Just one right now because this is mj and not gssj
	//No enemies; they are spawned dynamically.
	//TODO: Item locations.
};
struct __attribute__((packed)) Protocol_Msg_SC_GSSJOINRSP{
	//Map information, state, player locations/hp/etc, you
};
struct GameSessionData_Player{
	struct PositionOrientationVelocity pos;
	struct GameSessionData_Player_Inventory inv;
	double hp;
	bool hathshuffledoffthismortalcoil;
};
struct GameWorldState{
	struct GameSessionData_Player * players;
	int player_count;
};
enum WORLDUPDATE_TYPE{
	WORLDUPDATE_TYPE_playerPositionUpdate,
	WORLDUPDATE_TYPE_playerHealthUpdate,
	WORLDUPDATE_TYPE_playerJoinUpdate,
};
union __attribute__((packed)) GWU_U {
	struct __attribute__((packed)) {
		struct PositionOrientationVelocity pos;
		uint8_t player_no;
	} playerPositionUpdate;
	struct __attribute__((packed)) {
		double hp;
		bool hathshuffledoffthismortalcoil;
		uint8_t player_no;
	} playerHealthUpdate;
	struct GameSessionData_PLAYER_INIT playerJoinUpdate;
};
struct __attribute__((packed)) GWU_U_NT{
	enum WORLDUPDATE_TYPE type;
	union GWU_U update;
};
struct GameWorldUpdate{
	//In this, we have various types of updates (position, player join, etc). We need to serialise these in a way that the client can understand, and transmit them n times per second.
	enum WORLDUPDATE_TYPE type;
	union GWU_U update;
	struct GameWorldUpdate * next;
};
class GameSession{
	//TODO: polling groups, separate thread per game session
	public:
	struct GameWorldUpdate * update;
	int updlen;
	struct GameWorldState world;
	UserSession * sessions[255];
	uint8_t max_players;
	enum MAP_PUBLICITY publicity;
	uint64_t map_id;
	uint8_t difficulty;
	uint64_t join_code;
	void DistributeData();
	bool DDlock;
	bool dead;
	GameSession(uint8_t max_players, enum MAP_PUBLICITY publicity, uint64_t map_id, uint8_t difficulty){
		this->updlen = 0;
		this->update = NULL;
		this->world = GameWorldState();
		this->max_players = max_players;
		this->publicity = publicity;
		this->map_id = map_id;
		this->difficulty=difficulty;

		//Now: join code, player inventory.
	}
};

//For updates. Client may send us an update on e.g. position, and we need to check it and then distribute it in blocks.
void GameSession::DistributeData(){
	assert(!this->DDlock);
	this->DDlock=1;
	//construct it. This should rely on our updates as a constant from now on so that we don't get corruption, even if it might lead to latency.
	int playersno = this->world.player_count;
	struct GameWorldUpdate * k = this->update;
	if(!k){	
		assert(this->DDlock);
		this->DDlock=0;
		return;
	}
	struct GameWorldUpdate * k2 = k;
	int ulen = 1;
	while(k2->next){
		k2 = k2->next;
		ulen++;
	}
	struct GWU_U_NT * data = (struct GWU_U_NT*)malloc(ulen*sizeof(struct GWU_U_NT));
	int i = 0;
	while(k){
		data[i].type = k->type;
		data[i].update = k->update;
		i++;
		k = k->next;
	}
	//loop through every player and send them it
	for(int i = 0; i<playersno; i++){
		SERVER_INSTANCE.PH.sendCRQ(SC_WORLDUPDATE, getValidServerToken(), ulen*sizeof(struct GWU_U_NT), data, &(this->sessions[i]));
	}
	//TODO: add an ackpending to that session so we can add to the hsync counter later
	DD_END:
	assert(this->DDlock);
	this->DDlock=0;
}
//Next up, handling of the session specific updates from the client and the manager in general. Manager in general:
class GameSessionManager{
	public:
	GameSession ** sessions;
	int sessions_length; //no of elements (1)
	int sessions_size; //size in ptrs (1)
	bool manager_lock; //TODO: semaphores.
	void addSession(GameSession * k);
	void rmSession();
	void runUpdates();
	void commitSuicide();
};
void GameSessionManager::addSession(GameSession * k){
	assert(!this->manager_lock);
	this->manager_lock = true;
	if(this->sessions_length == this->sessions_size){
		this->sessions_size*=2;
		this->sessions = (GameSession**)realloc(this->sessions, this->sessions_size*sizeof(GameSession*));
	}
	this->sessions[this->sessions_length] = k;
	this->sessions_length++;
	assert(this->manager_lock);
	this->manager_lock = false;
}
void GameSessionManager::runUpdates(){
	int l = this->sessions_length;
	for(int i = 0; i<l; i++){
		if(this->sessions[i]->dead) continue;
		this->sessions[i]->DistributeData();
	}
}
enum PARSING_RESULT ProtocolHandler::handleCMSG(char * data, uint32_t packet_length, UserSession * ses_assoc){
	printf("%li\n", sizeof(struct Protocol_Msg));
	if(!data) return PR_GARBAGE_OURFAULT;
	if(packet_length < sizeof(struct Protocol_Msg)) return PR_WRONGPL;
	struct Protocol_Msg * pmsg = (struct Protocol_Msg*) data;
	char * ww = "WIZARDWEED";
	if(memcmp(ww, &(pmsg->wizardweed_notchecksum), 10)) return PR_GARBAGE; //Someone, non-client, is sending us garbage.
	printf("ml %i pl %i\n", pmsg->message_len, packet_length);
	if(pmsg->message_len != packet_length) return PR_PARTRECV;
	data = data+sizeof(struct Protocol_Msg);
	printf("rqrsppvr %X\n", pmsg->rqrspandpvr);
	if((pmsg->rqrspandpvr&0b111)!=PROTOCOL_VERSION) return PR_PROTOMISMATCH; 
	printf("1 ? protocol message passed.\n");
	switch(pmsg->type){
		case CS_AUTHRQ:
			//No token required. Handle login.
			//<Data> is (for authrq) a semipermanent token concatenated with a temporary symmetric key and encrypted with server pubkey - response is therefore a session token encrypted with that symmetric key.
#ifdef WIP_AUTH
			char * decrypted = unencryptSPK(data, pmsg->message_len-sizeof(struct Protocol_Msg));
			if(pmsg->message_len-sizeof(struct Protocol_Msg) != sizeof(struct Protocol_Msg_AuthRQ)) return PR_ILLOGICAL;
			struct Protocol_Msg_AuthRQ * rq = (struct Protocol_Msg_AuthRQ*)decrypted;
			if(validateSemiperma(rq->spt)){
				//Their semiperma is still valid. Give them a session token.
				struct Protocol_Msg_SC_TOKENRSP k;
				k.st = getTokenFromSemiperma(rq->spt);
				uint32_t encrypted_length = 0;
				char * encrypted = encryptSymmetric(k, sizeof(struct Protocol_Msg_SC_TOKENRSP), rq->sk, &encrypted_length);
				sendCRS(SC_TOKENRSP, getValidServerToken(), encrypted_length, pmsg->message_id_or_response_to, encrypted, ses_assoc);
				ses_assoc->session_user = getUserFromSemiperma(rq->spt);
				ses_assoc->session_token = k.st;
				free(encrypted);
				free(decrypted);
				return PR_SUCCESSFUL;
			}else{
				//Send them to the login screen.
				sendCRS(SC_LOGINRQ, getValidServerToken(), 0, pmsg->message_id_or_response_to, NULL, ses_assoc);
			}
			free(decrypted);
#endif
			return PR_SUCCESSFUL;
			break;
		case CS_LOGINRQ:
			//<Data> is (for loginrq) (char[128] USERNAME, char[128]PASSWORD) concatenated with a temporary symmetric key and encrypted with server pubkey - response is the same as above but also including a semipermanent token.
#ifdef WIP_AUTH
			char * decrypted = unencryptSPK(data, pmsg->message_len-sizeof(struct Protocol_Msg));
			if(pmsg->message_len-sizeof(struct Protocol_Msg) != sizeof(struct Protocol_Msg_LoginRQ)) return PR_ILLOGICAL;
			struct Protocol_Msg_LoginRQ * rq = (struct Protocol_Msg_LoginRQ*)decrypted;
			User * l_user = loginUser(rq.username, rq.password);
			if(!l_user){ //Invalid.	
				sendCRS(SC_LOGINRQ, getValidServerToken(), 0, pmsg->message_id_or_response_to, NULL, ses_assoc);
			}else{
				ses_assoc->session_user = l_user;
				struct Protocol_Msg_SC_TOKENRSPWSP k;
				k.spt = generateSemipermaFromUser(l_user);
				k.st = getTokenFromSemiperma(k.spt);
				ses_assoc->session_token = k.st;
				uint32_t encrypted_length = 0;
				char * encrypted = encryptSymmetric(k, sizeof(struct Protocol_Msg_SC_TOKENRSPWSP), rq->sk, &encrypted_length);
				sendCRS(SC_TOKENRSPWSP, getValidServerToken(), encrypted_length, pmsg->message_id_or_response_to, encrypted, ses_assoc);
				free(encrypted);
				free(decrypted);
				return PR_SUCCESSFUL;
			}
			free(decrypted);
#endif
			return PR_SUCCESSFUL;
			break;
		default:
			User u = ses_assoc->session_user;
			if(!u.user_id) return PR_ILLOGICAL; //Auth required first!
			if(pmsg->tokening){
				if(!verifyToken(pmsg->tokening, ses_assoc))
					return PR_TOKENINVALID;
			}else{
				return PR_ILLOGICAL;
			}
	}
	switch(pmsg->type){
		case CS_MAPJOINRQ: //Request to join map.
			struct Protocol_Msg_CS_MAPJOINRQ * rq;
		        rq = (struct Protocol_Msg_CS_MAPJOINRQ*)data;
			if(pmsg->message_len-sizeof(struct Protocol_Msg) != sizeof(struct Protocol_Msg_CS_MAPJOINRQ)) return PR_ILLOGICAL;
			//gotta make a new game session. need  a manager for game sessions??	

			break;
		case CS_GSSJOINRQ: //Request to join game session.
			break;
		default:
			return PR_ILLOGICAL;	
	}
	return PR_SUCCESSFUL;
}
void ProtocolHandler::sendCRQ(enum MESSAGE_TYPE t, ENCVAL_TEMP sctoken, uint32_t message_len, char * data, UserSession * ses_assoc){
	struct Protocol_Msg pmsg;
	char * ww = "WIZARDWEED";
	memcpy(pmsg.wizardweed_notchecksum,ww,10);
	pmsg.message_len=message_len+sizeof(struct Protocol_Msg);
	pmsg.rqrspandpvr=PROTOCOL_VERSION | 0b1000;
	memcpy(pmsg.tokening,sctoken, sizeof(ENCVAL_TEMP));
	pmsg.type = t;
	pmsg.message_id_or_response_to=current_message_id++;
	char * d = (char*)(malloc(pmsg.message_len));
	memcpy(d, &pmsg, sizeof(struct Protocol_Msg));
	memcpy(d+sizeof(struct Protocol_Msg), data, pmsg.message_len);
	ses_assoc->sendMessageTo(d, pmsg.message_len, k_nSteamNetworkingSend_Reliable); //TODO.
	free(d);
}
void ProtocolHandler::sendCRS(enum MESSAGE_TYPE t, ENCVAL_TEMP sctoken, uint32_t message_len, uint64_t response_to, char * data, UserSession * ses_assoc){
	struct Protocol_Msg pmsg;
	char * ww = "WIZARDWEED";
	memcpy(pmsg.wizardweed_notchecksum,ww,10);
	pmsg.message_len=message_len+sizeof(struct Protocol_Msg);
	pmsg.rqrspandpvr=PROTOCOL_VERSION & 0b111;
	memcpy(pmsg.tokening,sctoken, sizeof(ENCVAL_TEMP));
	pmsg.type = t;
	pmsg.message_id_or_response_to=response_to;
	char * d = (char*)malloc(pmsg.message_len);
	memcpy(d, &pmsg, sizeof(struct Protocol_Msg));
	memcpy(d+sizeof(struct Protocol_Msg), data, pmsg.message_len);
	ses_assoc->sendMessageTo(d, pmsg.message_len, k_nSteamNetworkingSend_Reliable); //TODO.
	free(d);

}
