#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include "Ticket.h"
#include "Database.h"
#include "hpdf.h"
#include "hpdf_barcode.h"
#include <libxml/parser.h>
#include <uuid/uuid.h>

class Config{
    public:
        Config();
        std::string name;
        std::string password;
        std::string user;
        std::string host;
        std::string time;
        std::string addr1;
        std::string addr2;
        std::string note;
        std::string logo;
};
Config::Config(){

}
void parseConfig(const char* filename, Config *conf){
    xmlDocPtr doc;
    xmlNodePtr cur;

    // Open the XML file
    doc = xmlReadFile(filename, NULL, XML_PARSE_NOBLANKS);

    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", filename);
        return;
    }

    // Get the root node
    cur = xmlDocGetRootElement(doc);

    // Check if the XML document is empty
    if (cur == NULL) {
        fprintf(stderr, "Empty XML document\n");
        xmlFreeDoc(doc);
        return;
    }

    // Check if the root node is a database tag
    if (xmlStrcmp(cur->name, (const xmlChar *) "config")) {
        fprintf(stderr, "Root node is not a config tag\n");
        xmlFreeDoc(doc);
        return;
    }
    cur = cur->xmlChildrenNode;
    if (xmlStrcmp(cur->name, (const xmlChar *) "database")) {
        fprintf(stderr, "Nested node is not a database tag\n");
        xmlFreeDoc(doc);
        return;
    }
    // Loop through the child nodes of the database tag
    for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
        // Check if the child node is a user tag
        if (!xmlStrcmp(cur->name, (const xmlChar *) "user")) {
            xmlChar* user = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->user = reinterpret_cast<char*>(user);
            xmlFree(user);
        }
        // Check if the child node is a password tag
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "password")) {
            xmlChar* password = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->password = reinterpret_cast<char*>(password);
            xmlFree(password);
        }
        // Check if the child node is a name tag
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "name")) {
            xmlChar* name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->name = reinterpret_cast<char*>(name);
            xmlFree(name);
        }
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "host")) {
            xmlChar* host = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->host = reinterpret_cast<char*>(host);
            xmlFree(host);
        }
    }
    // we end up with a NULL cursors so back to the beginning
    cur = xmlDocGetRootElement(doc);
    // we check the children then skip the first one which was the database
    cur = cur->xmlChildrenNode;
    cur = cur->next;
    if (xmlStrcmp(cur->name, (const xmlChar *) "ticket")) {
        fprintf(stderr, "Root node is not a ticket tag\n");
        xmlFreeDoc(doc);
        return;
    }
    for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
        // Check if the child node is a logo tag
        if (!xmlStrcmp(cur->name, (const xmlChar *) "logo")) {
            xmlChar* logo = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->logo = reinterpret_cast<char*>(logo);
            xmlFree(logo);
        }
        // Check if the child node is an address1 tag
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "address1")) {
            xmlChar* addr1 = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->addr1 = reinterpret_cast<char*>(addr1);
            xmlFree(addr1);
        }
        // Check if the child node is a time tag
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "time")) {
            xmlChar* time = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->time = reinterpret_cast<char*>(time);
            xmlFree(time);
        }
        // Check if the child node is a address2 tag
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "address2")) {
            xmlChar* addr2 = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->addr2 = reinterpret_cast<char*>(addr2);
            xmlFree(addr2);
        }
        else if (!xmlStrcmp(cur->name, (const xmlChar *) "note")) {
            xmlChar* note = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            conf->note = reinterpret_cast<char*>(note);
            xmlFree(note);
        }
    }

    // Free the memory used by the XML document
    xmlFreeDoc(doc);
    return;
}
void ticketUploader(Database *db, std::string filename){
    std::ifstream reader(filename);
    std::string ticketName;
    std::vector<Ticket> tickets;
     if(reader.is_open()){
        while(std::getline(reader, ticketName)){
            char ticketChar[37];
            uuid_t ticketUuid;
            // we create a uuid and then make it a char array lowercase (to be consistent)
            uuid_generate_random(ticketUuid);
            uuid_unparse_lower(ticketUuid, ticketChar);
            //std::cout << "Name: " << ticketName << "\tUUID: " << ticketChar << std::endl;
            std::string ticketStr{ticketChar};
            // we add the newly generated UUID and name to the list
            Ticket ticket = Ticket(ticketStr, ticketName);
            tickets.push_back(ticket);
        }
        reader.close();
    }
    else{
        std::cout << "Reading error" << std::endl;
    }
    // then we add the list into the database
    db->uploadTickets(tickets);
}
void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    std::cout << "ERROR: error_no=" << error_no << ", detail_no="<<  detail_no << std::endl;
    throw std::exception (); /* throw exception on error */
}
void pdfText(HPDF_Page page, HPDF_Font font, std::string text, int x, int y, int size, /* bool red, */ int justify){
    HPDF_Page_BeginText (page);
    HPDF_Page_SetFontAndSize (page, font, size);
    int tw = HPDF_Page_TextWidth(page, text.c_str());
    if(justify == 1){ // left justification
        HPDF_Page_MoveTextPos (page, x - tw, y);
    }
    else if(justify == 2){ // center justification
        HPDF_Page_MoveTextPos (page, x - (tw/2), y);
    }
    else{ // right justification
        HPDF_Page_MoveTextPos (page, x, y);
    }
    /* if(red)
        HPDF_Page_SetRGBFill(page, .9, .01, .01);
    else
        HPDF_Page_SetGrayFill(page, 0); */
    HPDF_Page_ShowText (page, text.c_str());
    HPDF_Page_EndText (page);
}
void ticketPrinter(std::vector<Ticket> tickets, HPDF_Doc pdf, HPDF_Page page, HPDF_Font fonts[], HPDF_Image logo, Config *conf){
    int leftRight = 60;
    int bottom, top;
    int width = 240;
    int height = 150;
    int ticketPlace;
    for(int i = 0; i < tickets.size(); i++){
        if(i % 10 == 0)
            page = HPDF_AddPage(pdf);
        ticketPlace = ((i % 10) / 2);
        if(i % 2)
            leftRight = 300;
        else
            leftRight = 60;
        bottom = HPDF_Page_GetHeight(page) - (height + 40) - ((ticketPlace)*height);
        top = bottom + height;
        HPDF_Page_SetGrayStroke(page, 0.9);
        HPDF_Page_Rectangle(page, leftRight, bottom, width, height);
        HPDF_Page_Stroke(page);
        HPDF_Page_DrawImage(page, logo, leftRight + width - 117, top - 42, 113, 40);
        pdfText(page, fonts[0], tickets[i].getName(), leftRight + 4, top - 16, 14,  0);
        pdfText(page, fonts[2], conf->time, leftRight + width - 4, top - 50, 12, 1);
        pdfText(page, fonts[1], conf->addr1, leftRight + width - 4, top - 68, 10, 1);
        pdfText(page, fonts[1], conf->addr2, leftRight + width - 4, top - 78, 10, 1);
        pdfText(page, fonts[3], conf->note, leftRight + width - 4, bottom + 8, 8, 1);
        hpdf_dmtx(page, tickets[i].getUuid().c_str(), leftRight + 5, bottom + 5, 2);
    }
}
int main(int argc, char **argv)
{
	Config *conf = new Config();
	parseConfig("config.xml", conf);
    Database *db = new Database(conf->host, conf->password, conf->user, conf->name);
    // if someone passes in a file, we'll use it as a tickets file
    if(argc > 1){
        ticketUploader(db, argv[1]);
    }
    HPDF_Doc pdf;
    pdf = HPDF_New(error_handler, NULL);
    HPDF_Page page;
    //page = HPDF_AddPage(pdf);
    const char* fontnm = HPDF_LoadTTFontFromFile(pdf, "Lora-Regular.ttf", HPDF_TRUE);
    const char* fontit = HPDF_LoadTTFontFromFile(pdf, "Lora-Italic.ttf", HPDF_TRUE);
    const char* fontbd = HPDF_LoadTTFontFromFile(pdf, "Lora-Bold.ttf", HPDF_TRUE);
    const char* fontmd = HPDF_LoadTTFontFromFile(pdf, "Lora-Medium.ttf", HPDF_TRUE);
    HPDF_Font fontReg, fontItalic, fontBold;
    HPDF_Font fonts[4];
    fonts[0] = HPDF_GetFont(pdf, fontnm, NULL);
    fonts[1] = HPDF_GetFont(pdf, fontit, NULL);
    fonts[2] = HPDF_GetFont(pdf, fontbd, NULL);
    fonts[3] = HPDF_GetFont(pdf, fontmd, NULL);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
    
    std::vector<Ticket> tickets = db->getTickets();
    HPDF_Image logo = HPDF_LoadPngImageFromFile(pdf, conf->logo.c_str());
    ticketPrinter(tickets, pdf, page, fonts, logo, conf);
    HPDF_STATUS result = HPDF_SaveToFile(pdf, "output.pdf");
    if(result == 4117){
        std::cout << "File open in other program" << std::endl;
    }
    //xmlFreeDoc(doc);
    delete db;
    HPDF_Free(pdf);
    return 0;
}
