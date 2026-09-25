#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <cassert>
#include <cstdio>
#include <thread>
#include <arpa/inet.h>
#include "headers/session.H"
#include "headers/protocol.H"
#include "headers/comm.H"
bool toTerminateServerProcess = false;
Server SERVER_INSTANCE;
int Server::doLiterallyEverything(uint32_t port){
	m_pInterface = SteamNetworkingSockets();
	SteamNetworkingIPAddr serverAddr;
	serverAddr.Clear();
	serverAddr.m_port = port;
	SteamNetworkingConfigValue_t cfg;
	cfg.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)GNS_SteamNetConnectionStatusChangedCallback);
	m_hListenSock = m_pInterface->CreateListenSocketIP(serverAddr, 1, &cfg);
	assert(m_hListenSock != k_HSteamListenSocket_Invalid);
	m_hPollGroup = m_pInterface->CreatePollGroup();
	assert(m_hPollGroup != k_HSteamNetPollGroup_Invalid);

	printf("Starting PI\n");
	while(!toTerminateServerProcess){
	//	printf("PCS\n");
		GNS_PollConnectionStatusChanges();
	//	printf("PI\n");
		GNS_PollIncoming();
					std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	}

	//TODO: send close messages and close all connections
	m_pInterface->CloseListenSocket(m_hListenSock);
	assert(m_pInterface->DestroyPollGroup(m_hPollGroup));
	return 1;
}

void Server::GNS_PollIncoming(){
	while(!toTerminateServerProcess){
		ISteamNetworkingMessage *incoming_msg = NULL;
		int n_msgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &incoming_msg, 1);
		if(n_msgs == 0) break;
		printf("we got something");
		assert(n_msgs==1 && incoming_msg); 
		printf("recv: %s\n", incoming_msg->m_pData);
		PH.handleCMSG((char*)(incoming_msg->m_pData), incoming_msg->m_cbSize, Sessions.findUserSessionByConnection(incoming_msg->m_conn));
		incoming_msg->Release();
	}
}
void Server::GNS_SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * change){
	//From docs: new connection, connection accepted by remote (will not happen), connection closed by remote, problem with connection (localhost closed)
	printf("change!!!\n");
	printf("start hexdump\n");
	for(long i = 0; i<sizeof(SteamNetConnectionStatusChangedCallback_t); i++){
		printf("%X", ((uint8_t*)(change))[i]);
	}
			printf("\ntheir ip btw is \n");
//			char * k = (char*)calloc(1,60);
//change.m_info.m_addrRemote.ToString(k, 60, 0);
//printf("%s\n",k);
	
//printf("is ipv4 is %i \n", change.m_info.m_addrRemote.IsIPv4());
//	printf("os %i\n", change.m_eOldState);
	switch(change->m_info.m_eState){
		case k_ESteamNetworkingConnectionState_None:
		case k_ESteamNetworkingConnectionState_Connected:
			printf("ch n or c\n");
			break;
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			//They unplugged their ethernet cable or we unplugged our ethernet cable. Act as a left game if in game.
			printf("PDL\n");
			if(!Sessions.removeUserSession(Sessions.findUserSessionByConnection(change->m_hConn))){
				assert(false);
			}
			m_pInterface->CloseConnection(change->m_hConn, 0, NULL, false);
			break;
		case k_ESteamNetworkingConnectionState_Connecting:
			printf("ch connecting\n");
			//Someone is trying to connect to us. Give them a session, then ask them for a token. If their token is valid, let them in, otherwise kick to login screen.
			if(m_pInterface->AcceptConnection(change->m_hConn) != k_EResultOK){ //Why can this even fail??
				m_pInterface->CloseConnection(change->m_hConn, 0, NULL, false);
				break;
			}
			if(!m_pInterface->SetConnectionPollGroup(change->m_hConn, m_hPollGroup)){
				m_pInterface->CloseConnection(change->m_hConn, 0, NULL, false);
				break;
			}
			Sessions.addUserSession(change->m_hConn);
			break;
		case k_ESteamNetworkingConnectionState_FindingRoute:
			printf("we should NOT be using the relay network\n");
			assert(false);
			break;
		case -1:

			printf("fws maybe\n");
			break;
		case 32767:
			printf("this is the fin wait state so we have disconnected on our side?\n");
			break;
		case 32766:
			printf("linger\n");
			break;
		case 32765:
			printf("connection DEAD\n");
			break;
		default:
			printf("Unknown change %i\n", change->m_info.m_eState);
			assert(false);	
	}
}
Server * s_pCallbackInstance=NULL;
void Server::GNS_SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t * change){ //Workaround. 
			/*										  	printf("RCV\n");
			printf("their ip btw is \n");
			char * k = (char*)calloc(1,49);
change.m_info.m_addrRemote.ToString(k, 49, 1);
printf("%s\n",k);*/
	s_pCallbackInstance->GNS_SteamNetConnectionStatusChanged(change);
}
void Server::GNS_PollConnectionStatusChanges(){
	s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
}
static void dotemp(ESteamNetworkingSocketsDebugOutputType type, const char *pmsg){
printf("%s\n", pmsg);
}
int main(int argc, char ** argv){
	SteamDatagramErrMsg errmsg;
	if(!GameNetworkingSockets_Init(NULL,errmsg)){
		return 1;
	}
	SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, dotemp );
	printf("Starting DLE\n");
	SERVER_INSTANCE.doLiterallyEverything(1263);

	//TODO: cleanup
	GameNetworkingSockets_Kill();
	return 0;
}
