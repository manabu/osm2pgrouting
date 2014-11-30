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

#ifndef OSMDOCUMENT_H
#define OSMDOCUMENT_H

#include "Configuration.h"

#include "leveldb/db.h"
#include <iostream>
#include <sstream>
#include <string>
#include <set>
namespace osm
{


class Node;
class Way;
class Relation;

/**
	An osm-document.
*/
class OSMDocument
{
public:
	//! Map, which saves the parsed nodes
	std::map<long long, Node*> m_Nodes;

	//! parsed ways
	std::vector<Way*> m_Ways;
	std::vector<long long> m_WaysIDs;
	//! splitted ways
	std::vector<Way*> m_SplittedWays;
	std::vector<long long> m_SplittedWaysIDs;

	std::vector<Relation*> m_Relations;
	std::vector<long long> m_RelationsIDs;


	Configuration& m_rConfig;
        //
        leveldb::DB* db;
        leveldb::WriteOptions writeOptions;
public:

	//! Constructor
	OSMDocument( Configuration& config);
	//! Destructor
	virtual ~OSMDocument();
	//! add node to the map
	void AddNode( Node* n );
	//! add node to the map
	void AddNode( long long id, double lat, double lon,unsigned short numsOfUse );
	//! add way to the map
	void AddWay( Way* w );
	//! add splitted way to the map
	void AddSplittedWay( Way* w );
	//! find node by using an ID
	Node* FindNode( long long nodeRefId ) const;
	//! find node by using an ID
	Node* FindNodeMain( long long nodeRefId ) const;
	//! find way by using an ID
	Way* FindWay( long long wayRefId ) const;
	//! find splitted way by using an ID
	Way* FindSplittedWay( long long wayRefId ) const;
	//! find way by using an ID
	Way* FindWayFromDB( long long wayRefId, char prefix ) const;
	//! find way by using an ID
	Way* FindWayFromDBMain( long long wayRefId, char prefix ) const;
	//! split the ways
	void SplitWays();
	//Node* getNode( long long nodeRefId );
	leveldb::Iterator * getDBIterator();
	//Node* FindNode(leveldb::Iterator *it);
	Node* convertToNode(std::string value) const;


	void AddRelation( Relation* r );
	//
	Relation* FindRelation(long long relationRefId) const;

};


} // end namespace osm
#endif
