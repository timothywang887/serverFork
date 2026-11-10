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



/*void GetSkillTreeInfo(UserSession * ses_assoc) {

  uint64_t id = ses_assoc->session_user.user_id;
  try {
    sql::Statement *stmt = con->createStatement();

    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM gameDB.player");
    while(res->next()) {
      std::cout << "ID: " << res->getInt("playerID");
      std::cout << ", Name: " << res->getString("username") << std::endl;
    }
    std::cout << "did in fact do a thing AGAIN" << std::endl;
  }

  catch (sql::SQLException &e) {
    std::cerr << "SQL ERROR: " << e.what() << std::endl;
  }
}
*/

void getidfromusername(char* input) {

    std::string searchTerm = input;
    std::string query = "SELECT * FROM player WHERE username = ?";
    sql::PreparedStatement* stmt = con->prepareStatement(query);
    stmt->setString(1, searchTerm);
    sql::ResultSet *res = stmt->executeQuery();
    
    while(res->next()) {
        std::cout << "PLID: " << res->getInt("playerID");
    }
}




int main(void) {

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
  char* input = "userthename";
  //GetSkillTreeInfo();
  getidfromusername(input);


}



