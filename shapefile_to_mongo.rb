#!/usr/bin/ruby

require 'rubygems'
require 'mongo'
require 'mysql'
require 'json'
include Mongo

db = 'gcsd000a10a_e'

mo   = Connection.new.db('civicsets')
coll = mo.collection('gcsd000a10a_e')
coll.remove()

my = Mysql::new("localhost", "root", "", 'gcsd000a10a_e')
my.query("SET CHARSET UTF8");
res = my.query("SELECT * FROM DBF");
fields = res.fetch_fields

while r = res.fetch_row do
  row = {}
  fields.each_with_index do |f,i|
    row[f.name] = r[i]
  end
  
  row['shape'] = []
  res2 = my.query("SELECT x,y FROM vertexes WHERE edge_id = #{r[0]} ORDER BY id");
  while vertex = res2.fetch_row do
    row['shape'] << vertex
  end

  coll.insert(row)
end
