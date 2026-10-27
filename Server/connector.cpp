#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include "headers/clientdata.H"


int main(void) {

  sql::mysql::MySQL_Driver *driver;
  sql::Connection *con;

  try {
    driver = sql::mysql::get_driver_instance();

    con = driver->connect("127.0.0.1:3306", "owner", "password");

    con->setSchema("gameDB");

    sql::Statement *stmt = con->createStatement();

    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM gameDB.player");

    while(res->next()) {
      std::cout << "ID: " << res->getInt("playerID");
      std::cout << ", Name: " << res->getString("username") << std::endl;
    }
    std::cout << "did in fact do a thing" << std::endl;

    delete res;
    delete stmt;
    delete con;

  }

  catch (sql::SQLException &e) {
    std::cerr << "SQL ERROR: " << e.what() << std::endl;


  }


}


void GetSkillTreeInfo() {




}
