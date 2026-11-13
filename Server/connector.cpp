#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <iostream>
#include "headers/clientdata.H"
#include "headers/session.H"

sql::mysql::MySQL_Driver *driver;
sql::Connection *con;


int getidfromusername(char* input) {

    std::string searchTerm = input;
    std::string query = "SELECT * FROM player WHERE username = ?";
    sql::PreparedStatement* stmt = con->prepareStatement(query);
    stmt->setString(1, searchTerm);
    sql::ResultSet *res = stmt->executeQuery();
    
    while(res->next()) {
        std::cout << "PLID: " << res->getInt("playerID");
	return res->getInt("playerID");
    }
    return 0;
}

GameSessionData_Player_Inventory DB_GetLoadoutFromUserSession(UserSession ses) {

        //build the struct to send
        ITEM_RELICS ir { ITEM_RELIC_CHEESE_SANDWICH, };
        ITEM_WEAPONS wp { ITEM_WEAPON_CHEESE_SANDWICH, };
//      Item_SkillTree tree =

        Item_WEAPON wpon;
        wpon.type = wp;
//      wpon.tree = tree;

        wpon.level = 12;
        wpon.xp = 0.643;

        GameSessionData_Player_Inventory send;
        send.weapon = wpon;
        send.relic = ir;

	std::string searchTerm = ses.session_user.username;
	std::string query = "SELECT * FROM player WHERE username = ?";
	sql::PreparedStatement* stmt = con->prepareStatement(query);
	stmt->setString(1, searchTerm);
	sql::ResultSet *res = stmt->executeQuery();

	while(res->next()) {
	    std::cout << "NEW FUNCTION LONG FUNCTIOPN THING: " << res->getInt("playerID");
	}

	std::cout << "WPON LEVEL: " << send.weapon.level << std::endl;
	return send;
}



int main(void) {



//EXAMPLE DATA FOR TESTING \/\/\/\/\/\/

User utest;
utest.user_id = 3302;
utest.username = "userthename";

UserSession ustest;
ustest.session_user = utest;

//CAN DELETE THIS LATER, EXAMPLE DATA ^^^^^^^^


  try {
    driver = sql::mysql::get_driver_instance();

    con = driver->connect("127.0.0.1:3306", "anewell", "password");

    con->setSchema("gameDB");

    sql::Statement *stmt = con->createStatement();

    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM gameDB.player");

    while(res->next()) {
      std::cout << "ID: " << res->getInt("playerID");
      std::cout << ", Name: " << res->getString("username") << std::endl;
    }
    std::cout << "did in fact do a thing" << std::endl;

  }

  catch (sql::SQLException &e) {
    std::cerr << "SQL ERROR: " << e.what() << std::endl;


  }

//After setup stuff

  char* input = "userthename";

  int plidtemp = getidfromusername(input);

  std::cout << "thisworks?: " << plidtemp << std::endl;

  DB_GetLoadoutFromUserSession(ustest);


}



