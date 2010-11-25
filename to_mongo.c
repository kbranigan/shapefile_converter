
#include "mongo.h"
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shapefil.h"

void bson_printf(bson * b)
{
  char hex_oid[25];
  bson_iterator it;
  bson_iterator_init(&it, b->data);
  while(bson_iterator_next(&it)){
    printf("  %s: ", bson_iterator_key(&it));

    switch(bson_iterator_type(&it)){
      case bson_double:
        printf("(double) %e\n", bson_iterator_double(&it));
        break;
      case bson_int:
        printf("(int) %d\n", bson_iterator_int(&it));
        break;
      case bson_string:
        printf("(string) \"%s\"\n", bson_iterator_string(&it));
        break;
      case bson_oid:
        bson_oid_to_string(bson_iterator_oid(&it), hex_oid);
        printf("(oid) \"%s\"\n", hex_oid);
        break;
      case bson_object:
        printf("(subobject) {...}\n");
        break;
      case bson_array:
        printf("(array) [...]\n");
        break;
      default:
        printf("(type %d)\n", bson_iterator_type(&it));
        break;
    }
  }
  printf("\n");
}

void cursor_printf(mongo_cursor * cursor)
{
  while (mongo_cursor_next(cursor))
  {
    bson_printf(&cursor->current);
  }
  mongo_cursor_destroy(cursor);
}

int main(int argc, char **argv)
{
  
  
  if (argc == 1) { printf("usage: shapefile_to_mongo [FILENAME]\n"); exit(1); }
  
  setlocale(LC_CTYPE, "en_CA.UTF-8");
  
  char file_name[150];
  sprintf(file_name, "%s", argv[1]);
    
  DBFHandle d = DBFOpen(file_name, "rb");
  if (d == NULL)
  {
    sprintf(file_name, "%s/%s", argv[1], argv[1]);
    d = DBFOpen(file_name, "rb");
    if (d == NULL)
    {
      printf("DBFOpen error (%s.dbf)\n", argv[1]);
      exit(1);
    }
  }
	
	SHPHandle h = SHPOpen(file_name, "rb");
  if (h == NULL)
  {
    printf("SHPOpen error (%s.dbf)\n", argv[1]);
    exit(1);
  }
	
  mongo_connection conn[1];
  mongo_connection_options opts;
  bson b;
  mongo_cursor * cursor;
  int i;
  
  strcpy(opts.host, "127.0.0.1");
  opts.port = 27017;
  
  if (mongo_connect(conn, &opts))
  {
    printf("failed to connect\n");
    exit(1);
  }
  
  int nRecordCount = DBFGetRecordCount(d);
  int nFieldCount = DBFGetFieldCount(d);
  printf("DBF has %d records (with %d fields)\n", nRecordCount, nFieldCount);
  
  int nShapeType;
  int nEntities;
  const char *pszPlus;
  double adfMinBound[4], adfMaxBound[4];
	
  SHPGetInfo(h, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
  printf("SHP has %d entities\n", nEntities);
  
  if (nRecordCount != nEntities) { printf("DBF and SHP dbs aren't the same.\n"); exit(1); }
  
  char ns[150];
  sprintf(ns, "civicsets.%s", argv[1]);
  printf("ns: %s\n", ns);
  
  bson_buffer *buf = (bson_buffer*)malloc(sizeof(bson_buffer));
  bson *p = (bson*)malloc(sizeof(bson));
  bson_buffer_init(buf);
  bson_from_buffer(p, buf);
  free(buf);
  mongo_remove(conn, (const char *)ns, p);
  
  for (int i = 0 ; i < nRecordCount ; i++)
  {
    buf = (bson_buffer*)malloc(sizeof(bson_buffer));
    p = (bson*)malloc(sizeof(bson));
    bson_buffer_init(buf);
    bson_append_new_oid(buf, "_id");
    
    SHPObject	*psShape = SHPReadObject(h, i);
    
    for (int j = 0 ; j < nFieldCount ; j++)
    {
      char pszFieldName[12];
      int pnWidth;
      int pnDecimals;
      
      DBFFieldType ft = DBFGetFieldInfo(d, j, pszFieldName, &pnWidth, &pnDecimals);
      if (ft == FTString)
      {
        const char * value = DBFReadStringAttribute(d, i, j);
        bson_append_string(buf, pszFieldName, value);
        printf("%s: %s\n", pszFieldName, value);
      }
    }
    
    /*bson_append_start_array(buf, "shape");
    for (int j = 0 ; j < psShape->nVertices ; j++)
    {
      char j_c[10];
      sprintf(j_c, "%d", j);
      bson_append_start_array(buf, j_c);
      bson_append_double(buf, "0", psShape->padfX[j]);
      bson_append_double(buf, "1", psShape->padfY[j]);
      bson_append_finish_object(buf);
    }
    bson_append_finish_object(buf);*/
    
    bson_from_buffer(p, buf);
    free(buf);
    
    mongo_insert(conn, (const char *)ns, p);
    
    bson_destroy(p);
    SHPDestroyObject(psShape);
    if (i == 1) break;
  }
}

