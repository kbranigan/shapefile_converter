
all:
	g++ main.c shpopen.o dbfopen.o -L/usr/local/mysql/lib/mysql -lmysqlclient -I/usr/local/mysql/include/mysql