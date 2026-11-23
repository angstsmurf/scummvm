/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/t/table.h"

/**************************************************
 *
 * USED IN:
 * Egypt: Voyage to the Land of the Pharaohs (Win, Mac)
 *
 **************************************************/

/*
-- xtra Table 
-- 
-- Table Xtra 
-- Version 1.0d13 
-- Copyright 1996 Electronic Ink 
-- 
new object 
+ register object, string serialNumber
 */

namespace Director {

const char *TableXtra::xlibName = "Table";
const XlibFileDesc TableXtra::fileNames[] = {
	{ "Table",   nullptr },
	{ "Table Xtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				TableXtra::m_new,		 0, 0,	700 },
	{ "register",				TableXtra::m_register,		 1, 1,	700 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

TableXtraObject::TableXtraObject(ObjectType ObjectType) :Object<TableXtraObject>("Table") {
	_objType = ObjectType;
}

bool TableXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum TableXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(TableXtra::xlibName);
	warning("TableXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void TableXtra::open(ObjectType type, const Common::Path &path) {
    TableXtraObject::initMethods(xlibMethods);
    TableXtraObject *xobj = new TableXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
        g_lingo->_openXtraObjects.push_back(xobj);
    }
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void TableXtra::close(ObjectType type) {
    TableXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void TableXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("TableXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(TableXtra::m_register, 0)

}
