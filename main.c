
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

const char *database = "shapefile_address_point";
const char *shpfile = "TCL3_ADDRESS_POINT";

int main()
{
  //int		nShapeType, nEntities, i, iPart, bValidate = 0,nInvalidCount=0;
  //const char 	*pszPlus;
  //double 	adfMinBound[4], adfMaxBound[4];
  
	DBFHandle d = DBFOpen(shpfile, "rb");
  if (d == NULL) { printf("DBFOpen error\n"); exit(1); }
	
  char filename[60];
  sprintf(filename, "%s.sql", database);
  printf("%s\n", filename);
  FILE *fp = fopen(filename, "w");
  if (fp == NULL) { printf("fopen error\n"); exit(1); }
	
  int nRecordCount = DBFGetRecordCount(d);
  int nFieldCount = DBFGetFieldCount(d);
  printf("DBF has %d records (with %d fields)\n", nRecordCount, nFieldCount);
	
  fprintf(fp, "DROP TABLE IF EXISTS DBF;\n");
  fprintf(fp, "CREATE TABLE DBF (id INT primary key auto_increment");
  for (int i = 0 ; i < nFieldCount ; i++)
  {
    char pszFieldName[12];
    int pnWidth;
    int pnDecimals;
    DBFFieldType ft = DBFGetFieldInfo(d, i, pszFieldName, &pnWidth, &pnDecimals);
    switch (ft){
      case FTString:
        fprintf(fp, ", %s VARCHAR(%d)", pszFieldName, pnWidth);
        break;
      case FTInteger:
        fprintf(fp, ", %s INT", pszFieldName);
        break;
      case FTDouble:
        fprintf(fp, ", %s FLOAT(15,10)", pszFieldName);
        break;
      case FTLogical:
        break;
      case FTInvalid:
        break;
    }
  }
  fprintf(fp, ");\n");
  
  for (int j = 0 ; j < nRecordCount ; j++)
  {
    char pszFieldName[12];
    int pnWidth;
    int pnDecimals;
    fprintf(fp, "INSERT INTO DBF VALUES (''");
    for (int i = 0 ; i < nFieldCount ; i++)
    {
      DBFFieldType ft = DBFGetFieldInfo(d, i, pszFieldName, &pnWidth, &pnDecimals);
      switch (ft){
        case FTString:
          fprintf(fp, ",\"%s\"", DBFReadStringAttribute(d, j, i));
          break;
        case FTInteger:
          fprintf(fp, ",\"%d\"", DBFReadIntegerAttribute(d, j, i));
          break;
        case FTDouble:
          fprintf(fp, ",\"%f\"", DBFReadDoubleAttribute(d, j, i));
          break;
        case FTLogical:
          break;
        case FTInvalid:
          break;
      }
    }
    fprintf(fp, ");\n");
    //printf("%s\n", temp_sql);
  }
	/*
	SHPHandle h = SHPOpen(shpfile, "rb");
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
	*/
}