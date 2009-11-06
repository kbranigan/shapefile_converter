
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"
#include "mysql.h"

// g++ main.c shpopen.o dbfopen.o -L/usr/local/mysql/lib/mysql -lmysqlclient -I/usr/local/mysql/include/mysql

int main()
{
  MYSQL mysql;
  MYSQL_RES res;
  MYSQL_ROW row;
  
  const char *database = "shapefile_address_point";
  const char *shpfile = "TCL3_ADDRESS_POINT";
  
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql, "localhost", "root", "", database, 0, NULL, 0));
  
  
  
	SHPHandle h;
  DBFHandle d;
	
  int		nShapeType, nEntities, i, iPart, bValidate = 0,nInvalidCount=0;
  const char 	*pszPlus;
  double 	adfMinBound[4], adfMaxBound[4];
  
	d = DBFOpen(shpfile, "rb");
	if (d == NULL) printf("error\n");
	
  int nRecordCount = DBFGetRecordCount(d);
  int nFieldCount = DBFGetFieldCount(d);
	
  printf("DBF nRecordCount = %d, nFieldCount = %d\n", nRecordCount, nFieldCount);
	
  mysql_query(&mysql, "DROP TABLE DBF");
  char temp_sql[5000];
  memset(temp_sql, 0, sizeof(temp_sql));
  strcat(temp_sql, "CREATE TABLE DBF (id INT primary key auto_increment");
  for (i = 0 ; i < nFieldCount ; i++)
  {
    char pszFieldName[12];
    int pnWidth;
    int pnDecimals;
    char tt[50];
    DBFFieldType ft = DBFGetFieldInfo(d, i, pszFieldName, &pnWidth, &pnDecimals);
    switch (ft){
      case FTString:
        sprintf(tt, ", %s VARCHAR(%d)", pszFieldName, pnWidth);
        strcat(temp_sql, tt);
      break;
      case FTInteger:
        sprintf(tt, ", %s INT", pszFieldName);
        strcat(temp_sql, tt);
      break;
      case FTDouble:
        sprintf(tt, ", %s FLOAT(15,10)", pszFieldName);
        strcat(temp_sql, tt);
      break;
    }
  }
  strcat(temp_sql, ");\n");
  mysql_query(&mysql, temp_sql);
  
  for (int j = 0 ; j < nRecordCount ; j++)
  {
    char pszFieldName[12];
    int pnWidth;
    int pnDecimals;
    const char *cVal;
    int iVal;
    double dVal;
    memset(temp_sql, 0, sizeof(temp_sql));
    strcat(temp_sql, "INSERT INTO DBF VALUES (''");
    for (i = 0 ; i < nFieldCount ; i++)
    {
      char tt[50];
      DBFFieldType ft = DBFGetFieldInfo(d, i, pszFieldName, &pnWidth, &pnDecimals);
      switch (ft){
        case FTString:
          cVal = DBFReadStringAttribute(d, j, i);
          //printf("%s VARCHAR(%d), ", pszFieldName, pnWidth);
          sprintf(tt, ",\"%s\"", cVal);
          strcat(temp_sql, tt);
        break;
        case FTInteger:
          iVal = DBFReadDoubleAttribute(d, j, i);
          //printf("%s INT, ", pszFieldName);
          sprintf(tt, ",\"%d\"", iVal);
          strcat(temp_sql, tt);
        break;
        case FTDouble:
          dVal = DBFReadDoubleAttribute(d, j, i);
          //printf("%s FLOAT(15,10), ", pszFieldName);
          sprintf(tt, ",\"%f\"", dVal);
          strcat(temp_sql, tt);
        break;
      }
    }
    strcat(temp_sql, ");\n");
    mysql_query(&mysql, temp_sql);
    //printf("%s\n", temp_sql);
  }
	
	h = SHPOpen(shpfile, "rb");
	if (h == NULL) printf("error\n");
	
  SHPGetInfo(h, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
  printf("Shapefile Type: %s   # of Shapes: %d\n", SHPTypeName(nShapeType), nEntities);
  //printf("File Bounds: (%12.3f,%12.3f,%g,%g)\n         to  (%12.3f,%12.3f,%g,%g)\n", adfMinBound[0], adfMinBound[1], adfMinBound[2], adfMinBound[3], adfMaxBound[0], adfMaxBound[1], adfMaxBound[2], adfMaxBound[3]);
  
  mysql_query(&mysql, "DROP TABLE edges");
  mysql_query(&mysql, "DROP TABLE vertexes");
  mysql_query(&mysql, "CREATE TABLE edges (id INT PRIMARY KEY AUTO_INCREMENT, name varchar(50))");
  mysql_query(&mysql, "CREATE TABLE vertexes (id INT PRIMARY KEY AUTO_INCREMENT, edge_id INT, x float(15,5), y float(15,5))");
  mysql_query(&mysql, "ALTER TABLE vertexes ADD KEY edge_id (edge_id)");
  for (i = 0; i < nEntities; i++)
  {
    int		j;
    SHPObject	*psShape;
    
    psShape = SHPReadObject(h, i);
    //printf("\nShape:%d (%s)  nVertices=%d, nParts=%d\n  Bounds:(%12.3f,%12.3f, %g, %g)\n      to (%12.3f,%12.3f, %g, %g)\n", i, SHPTypeName(psShape->nSHPType), psShape->nVertices, psShape->nParts, psShape->dfXMin, psShape->dfYMin, psShape->dfZMin, psShape->dfMMin, psShape->dfXMax, psShape->dfYMax, psShape->dfZMax, psShape->dfMMax );
    
    //mysql_query(&mysql, "INSERT INTO edges (name) VALUES ('')");
    int id = mysql_insert_id(&mysql);
    
    for (j = 0, iPart = 1; j < psShape->nVertices; j++)
    {
      const char *pszPartType = "";
      
      if (j == 0 && psShape->nParts > 0) pszPartType = SHPPartTypeName(psShape->panPartType[0]);
      if (iPart < psShape->nParts && psShape->panPartStart[iPart] == j)
      {
        pszPartType = SHPPartTypeName(psShape->panPartType[iPart]);
        iPart++;
        pszPlus = "+";
      }
      else
        pszPlus = " ";
      
      //char temp[500];
      //sprintf(temp, "INSERT INTO vertexes (edge_id, x, y) VALUES (%d, %f, %f)", id, psShape->padfX[j], psShape->padfY[j]);
      //mysql_query(&mysql, temp);
      
      //printf("  %s\n   %s (%f, %f) %s \n", temp, pszPlus, psShape->padfX[j], psShape->padfY[j], pszPartType);
    }
    
    SHPDestroyObject(psShape);
  }
  
	printf("all done\n");
	if (h != NULL) SHPClose(h);
	if (d != NULL) DBFClose(d);
	
	mysql_close(&mysql);
}