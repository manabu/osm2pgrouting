/***************************************************************************
 *   Copyright (C) 2008 by Daniel Wendt   								   *
 *   gentoo.murray@gmail.com   											   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "stdafx.h"
#include "OSMDocument.h"
#include "Configuration.h"
#include "Node.h"
#include "Relation.h"
#include "Way.h"
#include "math_functions.h"

#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#include <boost/algorithm/string.hpp>
#include <list>

#include "picojson.h"

long long countAddNode = 0;
long long countAddWay = 0;
long long countAddRelation = 0;
long long countFindNode = 0;
bool isWrite = true;
namespace osm
{

OSMDocument::OSMDocument( Configuration& config ) : m_rConfig( config )
{
  //writeOptions.sync = true;
leveldb::Options options;
options.create_if_missing = true;
leveldb::Status status = leveldb::DB::Open(options, "/usr/local/testdb2", &db);
assert(status.ok());


}

OSMDocument::~OSMDocument()
{
	ez_mapdelete( m_Nodes );
	ez_vectordelete( m_Ways );		
	ez_vectordelete( m_Relations );		
	ez_vectordelete( m_SplittedWays );
	/*
	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next())
	  {
	    std::cout << it->key().ToString() << " : " << it->value().ToString() << std::endl;
	  }
	*/
	delete db;
}
  leveldb::Iterator* OSMDocument::getDBIterator(){
    return db->NewIterator(leveldb::ReadOptions());
  }
void OSMDocument::AddNode( Node* n )
{
	AddNode(n->id, n->lat, n->lon , n->numsOfUse);
}
void OSMDocument::AddNode( long long id, double lat, double lon, unsigned short numsOfUse  ){
  //m_Nodes[n->id] = n;

	countAddNode++;
	if(countAddNode%100000==0){
	  std::cout << "AddNode count " << countAddNode << std::endl;
	}
	if(!isWrite){
	  return ;
	}
	std::ostringstream keyStream;
	//keyStream << "Key" << id;
	keyStream << "n" << id;
	std::ostringstream valueStream;
	//valueStream << "Test data value: " << n->id;
	valueStream << id << "," << std::setprecision(11) << lat << "," << lon << "," << numsOfUse;
	//std::cout << "add " << valueStream.str()  << std::endl;
	leveldb::Status s = db->Put(writeOptions, keyStream.str(), valueStream.str());
	if (!s.ok()) {
	  std::cout << "Error: [" <<  s.ToString() << "]" << std::endl;
	}
}
std::string WayToString(Way* w){
	//
	picojson::object o;
	picojson::array a;
	for (std::vector<Node*>::iterator it = w->m_NodeRefs.begin(); it != w->m_NodeRefs.end(); ++it) {
	  //delete *it;
	  Node* n = *it;
	  //std::cout << n->id << std::endl;
	  long long int id = n->id;
	  std::stringstream ss;
	  ss << id;
	  a.push_back(picojson::value(ss.str()));
	}
	o.insert(std::make_pair("m_NodeRefs", a));
	//
	picojson::object om;
	for (std::map<std::string, std::string>::iterator itpairstrstr = w->m_Tags.begin(); itpairstrstr != w->m_Tags.end(); itpairstrstr++) {
	  std::string strKey = itpairstrstr->first;
	  std::string    strVal   = itpairstrstr->second;
	  om.insert(std::make_pair(strKey, strVal));
	}
	o.insert(std::make_pair("m_Tags", om));
	// long long id
	std::stringstream ssid;
	ssid << w->id;
	o.insert(std::make_pair("id", ssid.str()));
	// visible
	o.insert(std::make_pair("visible", w->visible));
	// name
	o.insert(std::make_pair("name", w->name));
	// type
	o.insert(std::make_pair("type", w->type));
	// clss
	o.insert(std::make_pair("clss", w->clss));
	// geom
	o.insert(std::make_pair("geom", w->geom));
	// length
	o.insert(std::make_pair("length", w->length));
	// maxspeed_forward
	std::stringstream ssmaxspeed_forward;
	ssmaxspeed_forward << w->maxspeed_forward;
	o.insert(std::make_pair("maxspeed_forward", ssmaxspeed_forward.str()));
	// maxspeed_backward
	std::stringstream ssmaxspeed_backward;
	ssmaxspeed_backward << w->maxspeed_backward;
	o.insert(std::make_pair("maxspeed_backward", ssmaxspeed_backward.str()));
	// oneWayType
	std::string stroneWayType = "";
	switch(w->oneWayType){
	case YES:
	  stroneWayType = "YES";
	  break;
	case NO:
	  stroneWayType = "NO";
	  break;
	case REVERSED:
	  stroneWayType = "REVERSED";
	  break;
	}
	o.insert(std::make_pair("oneWayType", stroneWayType));
	// long long osm_id
	std::stringstream ssosm_id;
	ssosm_id << w->osm_id;
	o.insert(std::make_pair("osm_id", ssosm_id.str()));

	picojson::value v(o);
	//std::cout << v.serialize() << std::endl;

  std::string ret = v.serialize();
  return ret;
}

void OSMDocument::AddWay( Way* w )
{
  //m_Ways.push_back( w );
  m_WaysIDs.push_back( w->id );

	countAddWay++;
	if(countAddWay%10000==0){
	  std::cout << "AddWay count " << countAddWay << std::endl;
	}
	if(!isWrite){
	  return ;
	}

	//
	std::ostringstream keyStream;
	//keyStream << "Key" << n->id;
	keyStream << "w" << w->id;
	std::ostringstream valueStream;
	//valueStream << "Test data value: " << n->id;
	//valueStream << w->id << "," << n->lat << "," << n->lon << "," << n->numsOfUse;
	//std::cout << "add " << valueStream.str()  << std::endl;
	leveldb::Status s = db->Put(writeOptions, keyStream.str(), WayToString(w));
	if (!s.ok()) {
	  std::cout << "Error: [" << s.ToString() << "]" << std::endl;
	}


}
void OSMDocument::AddSplittedWay( Way* w )
{
  m_SplittedWaysIDs.push_back( w->id );
	if(!isWrite){
	  return ;
	}

	//
	std::ostringstream keyStream;
	//keyStream << "Key" << n->id;
	keyStream << "s" << w->id;
	std::ostringstream valueStream;
	//valueStream << "Test data value: " << n->id;
	//valueStream << w->id << "," << n->lat << "," << n->lon << "," << n->numsOfUse;
	//std::cout << "add " << valueStream.str()  << std::endl;
	leveldb::Status s = db->Put(writeOptions, keyStream.str(), WayToString(w));
	if (!s.ok()) {
	  std::cout << "Error: [" << s.ToString() << "]" << std::endl;
	}

}

void OSMDocument::AddRelation( Relation* r )
{
  //m_Relations.push_back( r );
  m_RelationsIDs.push_back( r->id );
	countAddRelation++;
	if(countAddRelation%1000==0){
	  std::cout << "AddRelation count " << countAddRelation << std::endl;
	}
	if(!isWrite){
	  return ;
	}
	//
	picojson::object o;
	picojson::array a;
	for (std::vector<long long>::iterator it = r->m_WayRefs.begin(); it != r->m_WayRefs.end(); ++it) {
	  std::stringstream ss;
	  ss << *it;
	  a.push_back(picojson::value(ss.str()));
	}
	o.insert(std::make_pair("m_WayRefs", a));
	//
	picojson::object om;
	for (std::map<std::string, std::string>::iterator itpairstrstr = r->m_Tags.begin(); itpairstrstr != r->m_Tags.end(); itpairstrstr++) {
	  std::string strKey = itpairstrstr->first;
	  std::string    strVal   = itpairstrstr->second;
	  om.insert(std::make_pair(strKey, strVal));
	}
	o.insert(std::make_pair("m_Tags", om));
	// long long id
	std::stringstream ssid;
	ssid << r->id;
	o.insert(std::make_pair("id", ssid.str()));
	// name
	o.insert(std::make_pair("name", r->name));

	picojson::value v(o);

	//
	std::ostringstream keyStream;
	//keyStream << "Key" << n->id;
	keyStream << "r" << r->id;
	std::ostringstream valueStream;
	//valueStream << "Test data value: " << n->id;
	//valueStream << w->id << "," << n->lat << "," << n->lon << "," << n->numsOfUse;
	//std::cout << "add " << valueStream.str()  << std::endl;
	leveldb::Status s = db->Put(writeOptions, keyStream.str(), v.serialize());
	if (!s.ok()) {
	  std::cout << "Error: [" << s.ToString() << "]" << std::endl;
	}

}

Node* OSMDocument::FindNode( long long nodeRefId ) 
const
{
  //std::map<long long, Node*>::const_iterator  it = m_Nodes.find( nodeRefId );
	countFindNode++;
	int retry=0;
	int MAX_RETRY=3;
	while(retry<=MAX_RETRY){
	  Node* node = FindNodeMain(nodeRefId);
	  if(node){
	    return node;
	  }else{
	    retry++;
	    std::cout << "Retry for nodeRef[" << nodeRefId << "] retry[" << retry << "/" << MAX_RETRY << "]" << std::endl;
	    sleep(1);
	  }
	}
	return 0;
}
Node*	OSMDocument::FindNodeMain(long long nodeRefId)
const
{
	std::string value;
	std::ostringstream keyStream;
	keyStream << "n" << nodeRefId;
	leveldb::Slice s1 = keyStream.str();

	leveldb::Status s = db->Get(leveldb::ReadOptions(), s1, &value);
	if(s.ok()){
	  return convertToNode(value);
	}else{
	  std::cout << "Error: [" << s.ToString() << "]" << std::endl;
	  std::cout << "NOT FOUND NODE [" << nodeRefId << "]" << std::endl;
	  return 0;
	}

	//return (it!=m_Nodes.end() ) ? it->second : 0;
}
Node* OSMDocument::convertToNode(std::string value)
const
{
	  //std::cout << "found" << nodeRefId << value << std::endl;
	  std::string delim(",");
	  std::list<std::string> list_string;
	  boost::split(list_string, value, boost::is_any_of(delim));
	  std::list<std::string>::iterator it = list_string.begin();
	  std::string id = *it;
	  it++;
	  std::string lat = *it;
	  it++;
	  std::string lon = *it;
	  it++;
	  std::string numsOfUse = *it;

	  //std::cout << "get " << id << " " << lat << " " << lon << std::endl;
	  //it++;
	  //return new Node(std::stoll(id), std::stod(lat), std::stod(lon));
	  std::stringstream ssid(id);
	  long long llid;
	  ssid >> llid;
	  std::stringstream sslat(lat);
	  double dlat;
	  sslat >> dlat;
	  std::stringstream sslon(lon);
	  double dlon;
	  sslon >> dlon;
	  std::stringstream ssnumsOfUse(numsOfUse);
	  unsigned short usnumsOfUse;
	  ssnumsOfUse >> usnumsOfUse;
	  Node* n = new Node(llid, dlat, dlon);
	  n->numsOfUse = usnumsOfUse;
	  return n;

  }
  Way* OSMDocument::FindWayFromDB( long long wayRefId, char prefix )
const
{
	int retry=0;
	int MAX_RETRY=3;
	while(retry<=MAX_RETRY){
	  Way* way = FindWayFromDBMain(wayRefId,prefix);
	  if(way){
	    return way;
	  }else{
	    retry++;
	    std::cout << "Retry for way[" << wayRefId << "] retry[" << retry << "/" << MAX_RETRY << "]" << std::endl;
	    sleep(1);
	  }
	}
	return 0;
}
Way* OSMDocument::FindWayFromDBMain( long long wayRefId, char prefix )
const
{
	std::string value;
	std::ostringstream keyStream;
	std::string prefixString;
	switch(prefix){
	case 's':
	  prefixString = "s";
	  break;
	case 'w':
	  prefixString = "w";
	  break;
	default:
	  prefixString = "w";
	  break;
	}
	keyStream << prefixString << wayRefId;
	leveldb::Slice s1 = keyStream.str();

	leveldb::Status s = db->Get(leveldb::ReadOptions(), s1, &value);
	if(s.ok()){
	  picojson::value json;
	  std::string err;
	  picojson::parse(json, value.begin(), value.end(), &err);
	  if (! err.empty()) {
	    std::cerr << err << std::endl;
	  }
	  picojson::object &o = json.get<picojson::object>();
	  // id
	  std::string str_id = o["id"].get<std::string>();
	  std::stringstream ssid(str_id);
	  long long id;
	  ssid >> id;
	  // visible
	  bool visible = o["visible"].get<bool>();
	  // name
	  std::string name = o["name"].get<std::string>();
	  // type
	  std::string type = o["type"].get<std::string>();
	  // clss
	  std::string clss = o["clss"].get<std::string>();
	  // geom
	  std::string geom = o["geom"].get<std::string>();
	  // length
	  double length = o["length"].get<double>();
	  // maxspeed_forward
	  std::string str_maxspeed_forward = o["maxspeed_forward"].get<std::string>();
	  std::stringstream ssmaxspeed_forward(str_maxspeed_forward);
	  int maxspeed_forward = 0;
	  ssmaxspeed_forward >> maxspeed_forward;
	  // maxspeed_forward
	  std::string str_maxspeed_backward = o["maxspeed_backward"].get<std::string>();
	  std::stringstream ssmaxspeed_backward(str_maxspeed_backward);
	  int maxspeed_backward = 0;
	  ssmaxspeed_backward >> maxspeed_backward;
	  // oneWayType
	  std::string stroneWayType = o["oneWayType"].get<std::string>();
	  OneWayType oneWayType;
	  if(stroneWayType=="YES"){
	    oneWayType = YES;
	  }else if(stroneWayType=="NO"){
	    oneWayType = NO;
	  }else if(stroneWayType=="REVERSED"){
	    oneWayType = REVERSED;
	  }
	  // osm_id
	  std::string str_osm_id = o["osm_id"].get<std::string>();
	  std::stringstream ssosm_id(str_osm_id);
	  long long osm_id;
	  ssosm_id >> osm_id;
	  //	Way( long long id, bool visible, long long osm_id,  int maxspeed_forward, int maxspeed_backward);

	  Way* w = new Way(id,visible,osm_id,maxspeed_forward,maxspeed_backward);
	  w->name = name;
	  w->type = type;
	  w->clss = clss;
	  w->geom = geom;
	  w->length = length;
	  w->oneWayType = oneWayType;
	  // m_NodeRefs
	  picojson::array& a = o["m_NodeRefs"].get<picojson::array>();
	  for (picojson::array::iterator it = a.begin(); it != a.end(); it++)
	    {
	      std::string strnid = it->get<std::string>();
	      std::stringstream ssnid(strnid);
	      long long nid;
	      ssnid >> nid;
	      Node* pNode = FindNode(nid);
	      if( pNode ) w->AddNodeRef(pNode);
	    }
	  // m_Tags
	  picojson::object om = o["m_Tags"].get<picojson::object>();
	  for (picojson::value::object::const_iterator it = om.begin(); it != om.end(); ++it) {
	    w->AddTag(it->first, it->second.to_str());
	  }
	//
	  return w;
	}else{
	  std::cout << "Error: [" << s.ToString() << "]" << std::endl;
	  std::cout << "NOT FOUND WAY [" << wayRefId << "]" << std::endl;
	  return 0;
	}

	//return (it!=m_Nodes.end() ) ? it->second : 0;
}
Way* OSMDocument::FindWay( long long wayRefId )
const
{
  char prefix = 'w';
  return FindWayFromDB(wayRefId, prefix);
}
Way* OSMDocument::FindSplittedWay( long long wayRefId ) 
const
{
  char prefix = 's';
  return FindWayFromDB(wayRefId, prefix);
}

Relation* OSMDocument::FindRelation( long long relationRefId ) 
const
{
	std::string value;
	std::ostringstream keyStream;
	keyStream << "r" << relationRefId;
	leveldb::Slice s1 = keyStream.str();

	leveldb::Status s = db->Get(leveldb::ReadOptions(), s1, &value);
	if(s.ok()){
	  picojson::value json;
	  std::string err;
	  picojson::parse(json, value.begin(), value.end(), &err);
	  if (! err.empty()) {
	    std::cerr << err << std::endl;
	  }
	  picojson::object &o = json.get<picojson::object>();
	  // id
	  std::string str_id = o["id"].get<std::string>();
	  std::stringstream ssid(str_id);
	  long long id;
	  ssid >> id;
	  //
	  Relation* r = new Relation( id );
	  // name
	  std::string name = o["name"].get<std::string>();
	  r->name = name;
	  // m_WayRefs
	  picojson::array& a = o["m_WayRefs"].get<picojson::array>();
	  for (picojson::array::iterator it = a.begin(); it != a.end(); it++)
	    {
	      std::string strwid = it->get<std::string>();
	      std::stringstream sswid(strwid);
	      long long wid;
	      sswid >> wid;
	      //Way* pWay = FindWay(wid);
	      //if( pWay ) r->AddWayRef(wid);
	      r->AddWayRef(wid);
	    }
	  // m_Tags
	  picojson::object om = o["m_Tags"].get<picojson::object>();
	  for (picojson::value::object::const_iterator it = om.begin(); it != om.end(); ++it) {
	    r->AddTag(it->first, it->second.to_str());
	  }
	  return r;
	}else{
	  std::cout << "Error: [" << s.ToString() << "]" << std::endl;
	  std::cout << "NOT FOUND RELATION [" << relationRefId << "]" << std::endl;
	  return 0;
	}

}
void OSMDocument::SplitWays()
{
	
  //std::vector<Way*>::const_iterator it(m_Ways.begin());
  //std::vector<Way*>::const_iterator last(m_Ways.end());
	std::vector<long long>::const_iterator it(m_WaysIDs.begin());
	std::vector<long long>::const_iterator last(m_WaysIDs.end());

	//splitted ways get a new ID
	long long id=0;
	// count how many process way
	long long countProcessWay = 0;
	while(it!=last)
	{
	countProcessWay++;
	if(countProcessWay%10000==0){
	  std::cout << "Process Way count " << countProcessWay << std::endl;
	  }

	  //Way* currentWay = *it++;
	  Way* currentWay = FindWay(*it++);
		
		// ITERATE THROUGH THE NODES
		std::vector<Node*>::const_iterator it_node( currentWay->m_NodeRefs.begin());	
		std::vector<Node*>::const_iterator last_node( currentWay->m_NodeRefs.end());
		
		Node* backNode = currentWay->m_NodeRefs.back();

		while(it_node!=last_node)
		{
			
		  //Node* node = *it_node++;
		  Node* node = new Node((*it_node)->id, (*it_node)->lat, (*it_node)->lon);
				  node->numsOfUse = (*it_node)->numsOfUse;
				  *it_node++;

			Node* secondNode=0;
			Node* lastNode=0;
			
			Way* splitted_way = new Way( ++id, currentWay->visible, currentWay->osm_id, currentWay->maxspeed_forward, currentWay->maxspeed_backward );
			splitted_way->name=currentWay->name;
			splitted_way->type=currentWay->type;
			splitted_way->clss=currentWay->clss;
			splitted_way->oneWayType=currentWay->oneWayType;
			
			std::map<std::string, std::string>::iterator it_tag( currentWay->m_Tags.begin() );
			std::map<std::string, std::string>::iterator last_tag( currentWay->m_Tags.end() );
//			std::cout << "Number of tags: " << currentWay->m_Tags.size() << std::endl;
//			std::cout << "First tag: " << currentWay->m_Tags.front()->key << ":" << currentWay->m_Tags.front()->value << std::endl;
		
			// ITERATE THROUGH THE TAGS
		
			while(it_tag!=last_tag)
			{
				std::pair<std::string, std::string> pair = *it_tag++;

				splitted_way->AddTag(pair.first, pair.second);
				
			}
			
			

	//GeometryFromText('LINESTRING('||x1||' '||y1||','||x2||' '||y2||')',4326);
			
			splitted_way->geom="LINESTRING("+ boost::lexical_cast<std::string>(node->lon) + " " + boost::lexical_cast<std::string>(node->lat) +",";
			
			splitted_way->AddNodeRef(node);
			
			bool found=false;
			
			if(it_node!=last_node)
			{
				while(it_node!=last_node && !found)
				{
				  //splitted_way->AddNodeRef(*it_node);
				  Node* newNode2 = new Node((*it_node)->id, (*it_node)->lat, (*it_node)->lon);
				  newNode2->numsOfUse = (*it_node)->numsOfUse;
				  splitted_way->AddNodeRef(newNode2);
					if((*it_node)->numsOfUse>1)
					{
						found=true;
						//secondNode = *it_node;
				  secondNode = new Node((*it_node)->id, (*it_node)->lat, (*it_node)->lon);
				  secondNode->numsOfUse = (*it_node)->numsOfUse;
						splitted_way->AddNodeRef(secondNode);

						double length = getLength(node,secondNode);
						if(length<0)
							length*=-1;
						splitted_way->length+=length;
						splitted_way->geom+= boost::lexical_cast<std::string>(secondNode->lon) + " " + boost::lexical_cast<std::string>(secondNode->lat) + ")";
						
					}
					else if(backNode->id==(*it_node)->id)
					{
					  //lastNode=*it_node++;
				  lastNode = new Node((*it_node)->id, (*it_node)->lat, (*it_node)->lon);
				  lastNode->numsOfUse = (*it_node)->numsOfUse;
				  *it_node++;
						splitted_way->AddNodeRef(lastNode);
						double length = getLength(node,lastNode);
						if(length<0)
							length*=-1;
						splitted_way->length+=length;
						splitted_way->geom+= boost::lexical_cast<std::string>(lastNode->lon) + " " + boost::lexical_cast<std::string>(lastNode->lat) + ")";
					}
					else
					{
						splitted_way->geom+= boost::lexical_cast<std::string>((*it_node)->lon) + " " + boost::lexical_cast<std::string>((*it_node)->lat) + ",";
						*it_node++;
					}
				}
			}
			if(splitted_way->m_NodeRefs.front()->id!=splitted_way->m_NodeRefs.back()->id){
			  //m_SplittedWays.push_back(splitted_way);
			  AddSplittedWay(splitted_way);
			  delete splitted_way;
				//splitted_way=0;
				splitted_way = NULL;
			}
			else
			{
			  delete splitted_way;
				//splitted_way=0;
				splitted_way = NULL;
			}
				
		}
		if(currentWay!=NULL){
		  delete currentWay;
		  //currentWay = 0;
		  currentWay = NULL;
		}
	}

} // end SplitWays

} // end namespace osm
