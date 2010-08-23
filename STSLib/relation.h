#ifndef RELATION_H
#define RELATION_H

/*
 *  This class will be used in class dsimsup.
 * 
 *  This class is similar to Su Rong's control relation. However, the difference is
 *  as following:
 * 	1. the relation is with respect to one single controllable event, not to all controllable events
 * 	2. the relation can be initialized to specify two types of marking relations.
 * 		marking type=SURONG: the computed marking DES has to synchronize with Plant to report marking.
 * 		marking type=INDEPENDENT: the computed marking DES can independently report marking.
 */

#include "bdd_synthesis.h"

bool const SURONG = true;
bool const INDEPENDENT  = false;

class relation {
	bdd	_enabled; // including dontcare states, marked in marking relations
	bdd	_disabled; // including dontcare states, not marked in marking relations
	bdd	_dontCare;
public:

	//constructors
	relation(bdd_synthesis const& syn, event const& e); // control relation constructor for e
	relation(bdd_synthesis const& syn, bool type = SURONG); // marking relation constructor
	
	//information
	bool operator ()(bdd const& ab) const // the set ab inside a cover of R
	{
		if( (ab & _enabled) == ab || (ab & _disabled) == ab ) // ab in a cell
			return true;
		else
			return false;
	}
	void split(bdd cell, vector<bdd>* const r) const;
	void getRemoved(bdd point, bdd cell, vector<bdd>* const r) const;
	void getEnlarged(bdd cell, vector<bdd>* const l, bdd excluded=bddfalse) const;
	pair<bdd,bdd> getCells() const { return make_pair(_enabled,_disabled); }
	bdd getDontCare()const { return _dontCare; }
	bdd getEnabled() const { return _enabled; }
	bdd getDisabled() const { return _disabled; }
	bdd getEnabledPart(bdd cell) const { return cell & _enabled; }
	bdd getUniqueEnabledPart(bdd cell) const { return getEnabledPart(cell)-_disabled; }
	bdd getDisabledPart(bdd cell) const { return cell& _disabled;}
	bdd getUniqueDisabledPart(bdd cell) const { return getDisabledPart(cell)-_enabled;}
	bdd getDontCarePart(bdd cell) const { return cell & getDontCare(); }
	bool isDontCare(bdd cell) const { return (cell & _dontCare) == cell; }
	bool isMustEnabled(bdd cell) const { return (cell & (_enabled-_disabled))!=bddfalse; }
	bool isMustDisabled(bdd cell) const { return (cell &(_disabled-_enabled)) != bddfalse; }
	int controlAction(bdd cell) const { // return: -1 must disabled, 0 dontcare, 1 must enabled. precond: cell must be control consistent
	  bdd me = _enabled - _disabled;
	  bdd md = _disabled- _enabled;
	  if( (cell & me) != bddfalse ) return 1;
	  else if( (cell & md) != bddfalse ) return -1;
	  else return 0;
	}
};

#endif /*RELATION_H*/
