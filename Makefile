
all: shpopen.o dbfopen.o
	g++ -Wall -O3 main.c shpopen.o dbfopen.o -o shapefile_to_mysqldump