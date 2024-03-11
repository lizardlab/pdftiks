#include "Ticket.h"

Ticket::Ticket(const std::string &pluid, const std::string &pname){
    luid = pluid;
    name = pname;
}
std::string Ticket::getName(){
    return name;
}
std::string Ticket::getUuid(){
    return luid;
}
