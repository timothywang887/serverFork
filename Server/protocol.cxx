#include "headers/protocol.H"
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
enum MAP_PUBLICITY{
	PUBLICITY_PRIVATE, //Only people with join code.
	PUBLICITY_FRIENDS, //Only friends.
	PUBLICITY_PUBLIC   //Everyone.
};
struct __attribute__((packed)) Protocol_Msg_CS_MAPJOINRQ{
	uint64_t map_id;
	enum MAP_PUBLICITY publicity;
	uint8_t difficulty_requested;
	uint8_t max_players;
};
struct __attribute__((packed)) Protocol_Msg_SC_MAPJOINRSP{//TODO
	//Map data
	//Session join code
	//Player position/inventory/etc data
	//Enemy position data
};
struct GameWorldState{
	//I don't know what will be in here. Enemies? Players? Structures?
};
struct GameSession{
	//TODO: polling groups, separate thread per game session
	struct GameWorldState world;
	UserSession * sessions[255];
	uint8_t max_players;
	enum MAP_PUBLICITY publicity;
	uint64_t map_id;
	uint8_t difficulty;
	uint64_t join_code;
};
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
