
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

int main(int argc, char **argv)
{
  if (argc == 1) { printf("usage: shapefile_to_mysqldump [FILENAME]\n"); exit(1); }
  
  DBFHandle d = DBFOpen(argv[1], "rb");
  if (d == NULL) { printf("DBFOpen error\n"); exit(1); }
	
  char filename[60];
  sprintf(filename, "%s.sql", argv[1]);
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
    switch (ft)
    {
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
  
  for (int i = 0 ; i < nRecordCount ; i++)
  {
    char pszFieldName[12];
    int pnWidth;
    int pnDecimals;
    fprintf(fp, "INSERT INTO DBF VALUES (''");
    for (int j = 0 ; j < nFieldCount ; j++)
    {
      DBFFieldType ft = DBFGetFieldInfo(d, j, pszFieldName, &pnWidth, &pnDecimals);
      switch (ft)
      {
        case FTString:
          fprintf(fp, ",\"%s\"", DBFReadStringAttribute(d, i, j));
          break;
        case FTInteger:
          fprintf(fp, ",\"%d\"", DBFReadIntegerAttribute(d, i, j));
          break;
        case FTDouble:
          fprintf(fp, ",\"%f\"", DBFReadDoubleAttribute(d, i, j));
          break;
        case FTLogical:
          break;
        case FTInvalid:
          break;
      }
    }
    fprintf(fp, ");\n");
  }
	
	SHPHandle h = SHPOpen(argv[1], "rb");
	if (h == NULL) printf("error\n");
	
  int nShapeType;
  int nEntities;
  const char *pszPlus;
  double adfMinBound[4], adfMaxBound[4];
	
  SHPGetInfo(h, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
  
  fprintf(fp, "DROP TABLE edges;\n");
  fprintf(fp, "DROP TABLE vertexes;\n");
  fprintf(fp, "CREATE TABLE edges (id INT PRIMARY KEY AUTO_INCREMENT);\n");
  fprintf(fp, "CREATE TABLE vertexes (id INT PRIMARY KEY AUTO_INCREMENT, edge_id INT, x float(15,5), y float(15,5));\n");
  fprintf(fp, "ALTER TABLE vertexes ADD KEY edge_id (edge_id);\n");
  for (int i = 0; i < nEntities; i++)
  {
    SHPObject	*psShape = SHPReadObject(h, i);
    
    fprintf(fp, "INSERT INTO edges (id) VALUES (%d);\n", i);
    
    for (int j = 0, iPart = 1; j < psShape->nVertices; j++)
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
      
      fprintf(fp, "INSERT INTO vertexes (edge_id, x, y) VALUES (%d, %f, %f);\n", i, psShape->padfX[j], psShape->padfY[j]);
    }
    
    SHPDestroyObject(psShape);
  }
  
	printf("all done\n");
	if (h != NULL) SHPClose(h);
	if (d != NULL) DBFClose(d);
}