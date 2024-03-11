#ifndef TICKET_H
#define TICKET_H
#include <string>

class Ticket{
    public:
        Ticket(const std::string &pluid, const std::string &name);
        std::string getUuid();
        std::string getName();

    private:
        std::string luid;
        std::string name;
};

#endif // TICKET_H
