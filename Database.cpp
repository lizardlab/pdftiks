#include "Database.h"
#include "mysql_connection.h"
#include "Ticket.h"


Database::Database(std::string connect, std::string password, std::string username, std::string db)
{
    connectStr = connect;
    user = username;
    try{
        driver = get_driver_instance();
        con = driver->connect(connect, username, password);
        con->setSchema(db);
    }
    catch(sql::SQLException &e){
        std::cout << "SQL Exception" << std::endl;
        std::cout << e.what() << std::endl;
    }
}
Database::~Database(){
    delete con;
    delete res;
    delete stmt;
}
std::vector<Ticket> Database::getTickets(){
    stmt = con->prepareStatement("SELECT * from `tickets`;");
    res = stmt->executeQuery();
    std::vector<Ticket> tickets;
    while (res->next()) {
        Ticket ticket = Ticket(res->getString("id").asStdString(), res->getString("name").asStdString());
        tickets.push_back(ticket);
    }
    return tickets;
}
void Database::uploadTickets(std::vector<Ticket> tickets){
    // first we create the table if it doesn't exist
    sql::Statement *st;
    st = con->createStatement();
    // we have extra columns which are used in the Flask version, just to keep things consistent
    st->execute("CREATE TABLE IF NOT EXISTS `tickets` (`id` char(36) NOT NULL, `name` varchar(255) DEFAULT NULL, `rsvp` tinyint(1) DEFAULT NULL, `revoked` tinyint(1) DEFAULT NULL, `scanned_time` datetime DEFAULT NULL, `phone` varchar(15) DEFAULT NULL, PRIMARY KEY (`id`), UNIQUE KEY `id` (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");
    // we set the RSVP and revoked to false to ensure no undefined behavior
    stmt = con->prepareStatement("INSERT INTO `tickets` (id, name, rsvp, revoked) VALUES (?, ?, 0, 0);"); 
    for(auto ticket: tickets){
        stmt->setString(1, ticket.getUuid());
        stmt->setString(2, ticket.getName());
        stmt->executeUpdate();
    }
    delete st;
}
