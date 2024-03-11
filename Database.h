#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <vector>
#include "Ticket.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

class Database
{
    public:
        Database(std::string connect, std::string username, std::string password, std::string db);
        ~Database();
        std::vector<Ticket> getTickets();
        void uploadTickets(std::vector<Ticket> tickets);
    protected:
    private:
        std::string connectStr;
        std::string user;
        std::string pass;
        std::string db;
        sql::Connection *con;
        sql::Driver *driver;
        sql::ResultSet *res;
        sql::PreparedStatement *stmt;
};

#endif // DATABASE_H
