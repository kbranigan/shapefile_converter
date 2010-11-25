
all: kml mysql

kml: shpopen.o dbfopen.o
	g++ -Wall -O3 to_kml.c shpopen.o dbfopen.o -o shapefile_to_kml

mysql: shpopen.o dbfopen.o
	g++ -Wall -O3 to_mysql.c shpopen.o dbfopen.o -o shapefile_to_mysqldump

mongo: shpopen.o dbfopen.o
	gcc --std=c99 -Imongo_files mongo_files/*.c to_mongo.c shpopen.o dbfopen.o -o shapefile_to_mongo
