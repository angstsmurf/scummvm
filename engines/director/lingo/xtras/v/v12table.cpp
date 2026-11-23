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
#include "director/lingo/xtras/v/v12dbe.h"
#include "director/lingo/xtras/v/v12table.h"

/**************************************************
 *
 * USED IN:
 * Egypt: Voyage to the Land of the Pharaohs (Win, Mac)
 *
 **************************************************/

/*
-- Xtra V12table 
-- part of V12 Database Engine 
-- ©Integration New Media, Inc. 1995-1999 
-- Please check the on-line help in the Xtras/V12-DBE menu 
new object me, integer db_object_ref, string tableName 
mAddRecord object me 
mAutoBinding object me, any Cast, * 
mBindField object me, string fieldname, long castmember, * 
mCustom object me, * 
mDataFormat object me, string fieldname, * 
mDeleteRecord object me 
mEditRecord object me 
mExportSelection object me, string outputtype, string destination, * 
mFind object me, string direction, * 
mGenerateMembers object me, * 
mGetField object me, string fieldname, * 
mGetMedia object me, string fieldname, * 
mGetPosition object me 
mGetProperty object me, string property 
mGetSelection object me, * 
mGetUnique object me, string outputtype 
mGo object me, integer position 
mGoFirst object me 
mGoLast object me 
mGoNext object me 
mGoPrevious object me 
mImport object me, * 
mImportFile object me, string inputtype, string source, * 
mNeedSelect object me 
mOrderBy object me, string fieldname, * 
mRefreshBoundFields object me, * 
mSelDelete object me 
mSelect object me, * 
mSelectAll object me 
mSelectCount object me 
mSetCriteria object me, * 
mSetField object me, string fieldname, * 
mSetIndex object me, string indexname 
mSetMedia object me, string fieldname, * 
mSetProperty object me, string property, string value 
mUnbindField object me, * 
mUpdateBoundFields object me, * 
mUpdateRecord object me 
 */

namespace Director {

const char *V12tableXtra::xlibName = "V12table";
const XlibFileDesc V12tableXtra::fileNames[] = {
	// Opened by V12dbeXtra, as both live in the same file
	{ "V12table",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				V12tableXtra::m_new,		 2, 2,	700 },
	{ "mAddRecord",				V12tableXtra::m_mAddRecord,		 0, 0,	700 },
	{ "mAutoBinding",				V12tableXtra::m_mAutoBinding,		 -1, 0,	700 },
	{ "mBindField",				V12tableXtra::m_mBindField,		 -1, 0,	700 },
	{ "mCustom",				V12tableXtra::m_mCustom,		 -1, 0,	700 },
	{ "mDataFormat",				V12tableXtra::m_mDataFormat,		 -1, 0,	700 },
	{ "mDeleteRecord",				V12tableXtra::m_mDeleteRecord,		 0, 0,	700 },
	{ "mEditRecord",				V12tableXtra::m_mEditRecord,		 0, 0,	700 },
	{ "mExportSelection",				V12tableXtra::m_mExportSelection,		 -1, 0,	700 },
	{ "mFind",				V12tableXtra::m_mFind,		 -1, 0,	700 },
	{ "mGenerateMembers",				V12tableXtra::m_mGenerateMembers,		 -1, 0,	700 },
	{ "mGetField",				V12tableXtra::m_mGetField,		 -1, 0,	700 },
	{ "mGetMedia",				V12tableXtra::m_mGetMedia,		 -1, 0,	700 },
	{ "mGetPosition",				V12tableXtra::m_mGetPosition,		 0, 0,	700 },
	{ "mGetProperty",				V12tableXtra::m_mGetProperty,		 1, 1,	700 },
	{ "mGetSelection",				V12tableXtra::m_mGetSelection,		 -1, 0,	700 },
	{ "mGetUnique",				V12tableXtra::m_mGetUnique,		 -1, 0,	700 },
	{ "mGo",				V12tableXtra::m_mGo,		 1, 1,	700 },
	{ "mGoFirst",				V12tableXtra::m_mGoFirst,		 0, 0,	700 },
	{ "mGoLast",				V12tableXtra::m_mGoLast,		 0, 0,	700 },
	{ "mGoNext",				V12tableXtra::m_mGoNext,		 0, 0,	700 },
	{ "mGoPrevious",				V12tableXtra::m_mGoPrevious,		 0, 0,	700 },
	{ "mImport",				V12tableXtra::m_mImport,		 -1, 0,	700 },
	{ "mImportFile",				V12tableXtra::m_mImportFile,		 -1, 0,	700 },
	{ "mNeedSelect",				V12tableXtra::m_mNeedSelect,		 0, 0,	700 },
	{ "mOrderBy",				V12tableXtra::m_mOrderBy,		 -1, 0,	700 },
	{ "mRefreshBoundFields",				V12tableXtra::m_mRefreshBoundFields,		 -1, 0,	700 },
	{ "mSelDelete",				V12tableXtra::m_mSelDelete,		 0, 0,	700 },
	{ "mSelect",				V12tableXtra::m_mSelect,		 -1, 0,	700 },
	{ "mSelectAll",				V12tableXtra::m_mSelectAll,		 0, 0,	700 },
	{ "mSelectCount",				V12tableXtra::m_mSelectCount,		 0, 0,	700 },
	{ "mSetCriteria",				V12tableXtra::m_mSetCriteria,		 -1, 0,	700 },
	{ "mSetField",				V12tableXtra::m_mSetField,		 -1, 0,	700 },
	{ "mSetIndex",				V12tableXtra::m_mSetIndex,		 1, 1,	700 },
	{ "mSetMedia",				V12tableXtra::m_mSetMedia,		 -1, 0,	700 },
	{ "mSetProperty",				V12tableXtra::m_mSetProperty,		 2, 2,	700 },
	{ "mUnbindField",				V12tableXtra::m_mUnbindField,		 -1, 0,	700 },
	{ "mUpdateBoundFields",				V12tableXtra::m_mUpdateBoundFields,		 -1, 0,	700 },
	{ "mUpdateRecord",				V12tableXtra::m_mUpdateRecord,		 0, 0,	700 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

V12tableXtraObject::V12tableXtraObject(ObjectType ObjectType) :Object<V12tableXtraObject>("V12table") {
	_objType = ObjectType;
}

bool V12tableXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum V12tableXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(V12tableXtra::xlibName);
	warning("V12tableXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

V12Table *V12tableXtraObject::table() {
	if (!_db || _tableIndex < 0 || _tableIndex >= (int)_db->tables.size())
		return nullptr;
	return &_db->tables[_tableIndex];
}

static int compareValues(const Datum &a, const Datum &b, bool isInteger) {
	if (isInteger) {
		int x = a.asInt(), y = b.asInt();
		return x < y ? -1 : (x > y ? 1 : 0);
	}
	return a.asString().compareToIgnoreCase(b.asString());
}

static bool matchCriterion(const Datum &v, const Common::String &op, const Datum &ref, bool isInteger) {
	if (op.equalsIgnoreCase("starts"))
		return v.asString().hasPrefixIgnoreCase(ref.asString());
	if (op.equalsIgnoreCase("contains"))
	{
		Common::String hay = v.asString(), needle = ref.asString();
		hay.toLowercase();
		needle.toLowercase();
		return hay.contains(needle);
	}
	int c = compareValues(v, ref, isInteger);
	if (op == "=")
		return c == 0;
	if (op == "<>" || op == "!=")
		return c != 0;
	if (op == "<")
		return c < 0;
	if (op == "<=")
		return c <= 0;
	if (op == ">")
		return c > 0;
	if (op == ">=")
		return c >= 0;
	warning("V12tableXtra: unsupported criteria operator '%s'", op.c_str());
	return false;
}

bool V12tableXtraObject::matches(const Common::Array<Datum> &record) const {
	bool result = true;
	for (uint i = 0; i < _criteria.size(); i++) {
		const Criterion &c = _criteria[i];
		bool isInteger = _db->tables[_tableIndex].fields[c.field].type == kV12Integer;
		bool m = matchCriterion(record[c.field], c.op, c.value, isInteger);
		if (i == 0)
			result = m;
		else if (c.conjunction.equalsIgnoreCase("OR"))
			result = result || m;
		else
			result = result && m;
	}
	return result;
}

void V12tableXtraObject::select() {
	_selection.clear();
	V12Table *t = table();
	if (!t) {
		_pos = -1;
		return;
	}
	for (uint i = 0; i < t->records.size(); i++)
		if (matches(t->records[i]))
			_selection.push_back(i);
	if (!_order.empty()) {
		Common::sort(_selection.begin(), _selection.end(), [&](uint a, uint b) {
			for (auto &k : _order) {
				int c = compareValues(t->records[a][k.field], t->records[b][k.field], t->fields[k.field].type == kV12Integer);
				if (c)
					return k.descending ? c > 0 : c < 0;
			}
			return a < b;
		});
	}
	_pos = _selection.empty() ? -1 : 0;
	_editMode = kNotEditing;
}

Common::Array<Datum> *V12tableXtraObject::currentRecord() {
	V12Table *t = table();
	if (!t || _pos < 0 || _pos >= (int)_selection.size())
		return nullptr;
	return &t->records[_selection[_pos]];
}

void V12tableXtra::open(ObjectType type, const Common::Path &path) {
	V12tableXtraObject::initMethods(xlibMethods);
	V12tableXtraObject *xobj = new V12tableXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void V12tableXtra::close(ObjectType type) {
	V12tableXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

#define V12_ME V12tableXtraObject *me = static_cast<V12tableXtraObject *>(g_lingo->_state->me.u.obj)

// new(xtra "V12table", mGetRef(db), tableName)
void V12tableXtra::m_new(int nargs) {
	Common::String tableName = g_lingo->pop().asString();
	int ref = g_lingo->pop().asInt();
	V12_ME;
	me->_db = V12dbeXtra::getDatabase(ref);
	if (!me->_db) {
		V12dbeXtra::setError(-3, Common::String::format("Invalid database reference %d", ref));
		g_lingo->push(Datum(-3));
		return;
	}
	me->_tableIndex = -1;
	for (uint i = 0; i < me->_db->tables.size(); i++)
		if (me->_db->tables[i].name.equalsIgnoreCase(tableName))
			me->_tableIndex = i;
	if (me->_tableIndex < 0) {
		V12dbeXtra::setError(-4, Common::String::format("Table \"%s\" not found in %s", tableName.c_str(), me->_db->fileName.c_str()));
		g_lingo->push(Datum(-4));
		return;
	}
	me->_criteria.clear();
	me->_order.clear();
	me->select();
	V12dbeXtra::setError(0, Common::String());
	g_lingo->push(g_lingo->_state->me);
}

static int fieldIndex(V12tableXtraObject *me, const Common::String &name, const char *method) {
	V12Table *t = me->table();
	int f = t ? t->findField(name) : -1;
	if (f < 0)
		V12dbeXtra::setError(-5, Common::String::format("%s: field \"%s\" not found in table %s", method, name.c_str(), t ? t->name.c_str() : "?"));
	return f;
}

// mGetField(table, fieldName {, dateFormat})
void V12tableXtra::m_mGetField(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	Common::String name = g_lingo->pop().asString();
	V12_ME;
	int f = fieldIndex(me, name, "mGetField");
	if (f < 0) {
		g_lingo->push(Datum(-5));
		return;
	}
	if (me->_editMode != V12tableXtraObject::kNotEditing) {
		g_lingo->push(me->_buffer[f]);
		return;
	}
	Common::Array<Datum> *rec = me->currentRecord();
	if (!rec) {
		V12dbeXtra::setError(-6, "mGetField: no current record");
		V12Table *t = me->table();
		g_lingo->push(t->fields[f].type == kV12Integer ? Datum(0) : Datum(Common::String()));
		return;
	}
	g_lingo->push((*rec)[f]);
}

// mSetField(table, fieldName, value {, format})
void V12tableXtra::m_mSetField(int nargs) {
	if (nargs > 2)
		g_lingo->dropStack(nargs - 2);
	Datum value = g_lingo->pop();
	Common::String name = g_lingo->pop().asString();
	V12_ME;
	int f = fieldIndex(me, name, "mSetField");
	if (f < 0) {
		g_lingo->push(Datum(-5));
		return;
	}
	if (me->_editMode == V12tableXtraObject::kNotEditing) {
		V12dbeXtra::setError(-7, "mSetField: record not in edit mode");
		g_lingo->push(Datum(-7));
		return;
	}
	me->_buffer[f] = me->table()->fields[f].type == kV12Integer ? Datum(value.asInt()) : Datum(value.asString());
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mEditRecord(int nargs) {
	V12_ME;
	Common::Array<Datum> *rec = me->currentRecord();
	if (!rec) {
		V12dbeXtra::setError(-6, "mEditRecord: no current record");
		g_lingo->push(Datum(-6));
		return;
	}
	me->_buffer = *rec;
	me->_editMode = V12tableXtraObject::kEditing;
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mAddRecord(int nargs) {
	V12_ME;
	V12Table *t = me->table();
	if (!t) {
		g_lingo->push(Datum(-4));
		return;
	}
	me->_buffer = t->blankRecord();
	me->_editMode = V12tableXtraObject::kAdding;
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mUpdateRecord(int nargs) {
	V12_ME;
	V12Table *t = me->table();
	if (!t || me->_editMode == V12tableXtraObject::kNotEditing) {
		V12dbeXtra::setError(-7, "mUpdateRecord: record not in edit mode");
		g_lingo->push(Datum(-7));
		return;
	}
	if (!me->_db->readWrite) {
		V12dbeXtra::setError(-8, "mUpdateRecord: database is read-only");
		me->_editMode = V12tableXtraObject::kNotEditing;
		g_lingo->push(Datum(-8));
		return;
	}
	if (me->_editMode == V12tableXtraObject::kAdding) {
		t->records.push_back(me->_buffer);
		me->_selection.push_back(t->records.size() - 1);
		me->_pos = me->_selection.size() - 1;
	} else if (Common::Array<Datum> *rec = me->currentRecord()) {
		*rec = me->_buffer;
	}
	me->_editMode = V12tableXtraObject::kNotEditing;
	me->_db->saveChanges();
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mDeleteRecord(int nargs) {
	V12_ME;
	V12Table *t = me->table();
	if (!me->currentRecord()) {
		V12dbeXtra::setError(-6, "mDeleteRecord: no current record");
		g_lingo->push(Datum(-6));
		return;
	}
	if (!me->_db->readWrite) {
		V12dbeXtra::setError(-8, "mDeleteRecord: database is read-only");
		g_lingo->push(Datum(-8));
		return;
	}
	uint idx = me->_selection[me->_pos];
	t->records.remove_at(idx);
	me->_selection.remove_at(me->_pos);
	for (auto &s : me->_selection)
		if (s > idx)
			s--;
	if (me->_pos >= (int)me->_selection.size())
		me->_pos = (int)me->_selection.size() - 1;
	me->_editMode = V12tableXtraObject::kNotEditing;
	me->_db->saveChanges();
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mSelectAll(int nargs) {
	V12_ME;
	me->_criteria.clear();
	me->_order.clear();
	me->select();
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mSelect(int nargs) {
	g_lingo->dropStack(nargs);
	V12_ME;
	me->select();
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mSelectCount(int nargs) {
	V12_ME;
	g_lingo->push(Datum((int)me->_selection.size()));
}

// mSetCriteria(table, {"AND" | "OR",} fieldName, operator, value)
void V12tableXtra::m_mSetCriteria(int nargs) {
	Common::Array<Datum> args;
	for (int i = 0; i < nargs; i++)
		args.insert_at(0, g_lingo->pop());
	V12_ME;
	V12tableXtraObject::Criterion c;
	uint a = 0;
	if (args.size() == 4) {
		c.conjunction = args[a++].asString();
	} else if (args.size() != 3) {
		warning("V12tableXtra::m_mSetCriteria: unsupported form with %d arguments", nargs);
		g_lingo->push(Datum(-9));
		return;
	}
	c.field = fieldIndex(me, args[a].asString(), "mSetCriteria");
	c.op = args[a + 1].asString();
	c.value = args[a + 2];
	if (c.conjunction.empty())
		me->_criteria.clear();
	if (c.field < 0) {
		g_lingo->push(Datum(-5));
		return;
	}
	me->_criteria.push_back(c);
	g_lingo->push(Datum(0));
}

// mOrderBy(table, fieldName {, "ascending" | "descending"})
void V12tableXtra::m_mOrderBy(int nargs) {
	Common::String direction = nargs >= 2 ? g_lingo->pop().asString() : Common::String();
	if (nargs > 2)
		g_lingo->dropStack(nargs - 2);
	Common::String name = g_lingo->pop().asString();
	V12_ME;
	int f = fieldIndex(me, name, "mOrderBy");
	if (f < 0) {
		g_lingo->push(Datum(-5));
		return;
	}
	me->_order.push_back({f, direction.equalsIgnoreCase("descending")});
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mGetPosition(int nargs) {
	V12_ME;
	g_lingo->push(Datum(me->_pos + 1));
}

static void goTo(V12tableXtraObject *me, int pos) {
	if (pos < 0 || pos >= (int)me->_selection.size()) {
		V12dbeXtra::setError(-10, Common::String::format("Position %d is outside the selection (%d records)", pos + 1, me->_selection.size()));
		g_lingo->push(Datum(-10));
		return;
	}
	me->_pos = pos;
	me->_editMode = V12tableXtraObject::kNotEditing;
	g_lingo->push(Datum(0));
}

void V12tableXtra::m_mGo(int nargs) {
	int pos = g_lingo->pop().asInt();
	V12_ME;
	goTo(me, pos - 1);
}

void V12tableXtra::m_mGoFirst(int nargs) {
	V12_ME;
	goTo(me, 0);
}

void V12tableXtra::m_mGoLast(int nargs) {
	V12_ME;
	goTo(me, (int)me->_selection.size() - 1);
}

void V12tableXtra::m_mGoNext(int nargs) {
	V12_ME;
	goTo(me, me->_pos + 1);
}

void V12tableXtra::m_mGoPrevious(int nargs) {
	V12_ME;
	goTo(me, me->_pos - 1);
}

// mGetUnique(table, outputType {, fieldName {, order}}): the distinct values
// of a field in the selection. Without a field name, use the sort field.
void V12tableXtra::m_mGetUnique(int nargs) {
	Common::Array<Datum> args;
	for (int i = 0; i < nargs; i++)
		args.insert_at(0, g_lingo->pop());
	V12_ME;
	V12Table *t = me->table();
	int f = -1;
	if (args.size() >= 2)
		f = fieldIndex(me, args[1].asString(), "mGetUnique");
	else if (!me->_order.empty())
		f = me->_order[0].field;
	else if (t && !t->fields.empty())
		f = 0;
	Datum res;
	res.type = ARRAY;
	res.u.farr = new FArray;
	if (f >= 0) {
		for (uint idx : me->_selection) {
			const Datum &v = t->records[idx][f];
			bool seen = false;
			for (auto &d : res.u.farr->arr)
				if (compareValues(d, v, t->fields[f].type == kV12Integer) == 0)
					seen = true;
			if (!seen)
				res.u.farr->arr.push_back(v);
		}
	}
	if (!args.empty() && !args[0].asString().equalsIgnoreCase("List")) {
		Common::String s;
		for (auto &d : res.u.farr->arr)
			s += d.asString() + "\r";
		g_lingo->push(Datum(s));
		return;
	}
	g_lingo->push(res);
}

// mGetSelection(table {, "Literal" | "List" {, from, count, fieldDelim, recordDelim}})
void V12tableXtra::m_mGetSelection(int nargs) {
	Common::Array<Datum> args;
	for (int i = 0; i < nargs; i++)
		args.insert_at(0, g_lingo->pop());
	V12_ME;
	V12Table *t = me->table();
	bool list = !args.empty() && args[0].asString().equalsIgnoreCase("List");
	Datum res;
	if (list) {
		res.type = ARRAY;
		res.u.farr = new FArray;
	}
	Common::String s;
	for (uint idx : me->_selection) {
		if (list) {
			Datum row;
			row.type = ARRAY;
			row.u.farr = new FArray;
			row.u.farr->arr = t->records[idx];
			res.u.farr->arr.push_back(row);
		} else {
			for (uint f = 0; f < t->fields.size(); f++)
				s += t->records[idx][f].asString() + (f + 1 < t->fields.size() ? "\t" : "\r");
		}
	}
	g_lingo->push(list ? res : Datum(s));
}

void V12tableXtra::m_mNeedSelect(int nargs) {
	g_lingo->push(Datum(0));
}

XOBJSTUB(V12tableXtra::m_mAutoBinding, 0)
XOBJSTUB(V12tableXtra::m_mBindField, 0)
XOBJSTUB(V12tableXtra::m_mCustom, 0)
XOBJSTUB(V12tableXtra::m_mDataFormat, 0)
XOBJSTUB(V12tableXtra::m_mExportSelection, 0)
XOBJSTUB(V12tableXtra::m_mFind, 0)
XOBJSTUB(V12tableXtra::m_mGenerateMembers, 0)
XOBJSTUB(V12tableXtra::m_mGetMedia, 0)
XOBJSTUB(V12tableXtra::m_mGetProperty, 0)
XOBJSTUB(V12tableXtra::m_mImport, 0)
XOBJSTUB(V12tableXtra::m_mImportFile, 0)
XOBJSTUB(V12tableXtra::m_mRefreshBoundFields, 0)
XOBJSTUB(V12tableXtra::m_mSelDelete, 0)
XOBJSTUB(V12tableXtra::m_mSetIndex, 0)
XOBJSTUB(V12tableXtra::m_mSetMedia, 0)
XOBJSTUB(V12tableXtra::m_mSetProperty, 0)
XOBJSTUB(V12tableXtra::m_mUnbindField, 0)
XOBJSTUB(V12tableXtra::m_mUpdateBoundFields, 0)

}
