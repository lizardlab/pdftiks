pdftiks: pdftiks.cpp Ticket.cpp Database.cpp hpdf_barcode.c pdf417lib.c
	g++ -g -c Ticket.cpp Database.cpp hpdf_barcode.c pdf417lib.c
	g++ -g -o pdftiks pdftiks.cpp Ticket.o Database.o hpdf_barcode.o pdf417lib.o `xml2-config --cflags` `xml2-config --libs` `pkg-config --libs libdmtx libqrencode uuid zlib` -lhpdf -lmysqlcppconn

clean:
	rm pdftiks Ticket.o Database.o hpdf_barcode.o pdf417lib.o
