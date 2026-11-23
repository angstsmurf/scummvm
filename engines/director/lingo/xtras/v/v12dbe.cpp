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

#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/v/v12dbe.h"
#include "director/lingo/xtras/v/v12table.h"
#include "director/util.h"

/**************************************************
 *
 * USED IN:
 * Egypt: Voyage to the Land of the Pharaohs (Win, Mac)
 *
 **************************************************/

/*
-- Xtra V12dbe 
-- part of V12 Database Engine 
-- ©Integration New Media, Inc. 1995-1999 
-- Please check the on-line help in the Xtras/V12-DBE menu 
new object me, string databasename, string openmode, * 
 mBuild object me, * 
mCloneDatabase object me, string databasename 
mCreateField object me, string tablename, string fieldname, * 
mCreateFullIndex object me, string tablename, string fieldname, * 
mCreateIndex object me, string tablename, string indexname, string isunique, string fieldname, string order, * 
mCreateTable object me, string tablename 
mCustom object me, * 
mDeleteTable object me, string tablename 
mDumpStructure object me, * 
mEditDBStructure object me 
+ mError object xtraRef, * 
+ mFixDatabase object xtraRef, string databasename, string newdatabasename 
mGetPropertyNames object me, * 
mGetProperty object me, string property 
mGetRef object me 
mPackDatabase object me, string newdatabasename 
mReadDBStructure object me, string inputtype, string source, * 
mRenameField object me, string tablename, string oldfieldname, string newfieldname 
mSetPassword object me, string oldpassword, string newpassword 
mSetProperty object me, string property, string value 
+ mStatus object xtraRef 
mUpdateDBStructure object me 
* V12Error *
* V12ErrorReset 
* V12Status 
+ mXtraVersion object xtraRef 
 */

namespace Director {

const char *V12dbeXtra::xlibName = "V12dbe";
const XlibFileDesc V12dbeXtra::fileNames[] = {
	{ "V12-DBE for Director",   nullptr },
	{ "V12-DBE for Director.Xtr",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				V12dbeXtra::m_new,		 -1, 0,	700 },
	{ "mBuild",				V12dbeXtra::m_mBuild,		 -1, 0,	700 },
	{ "mCloneDatabase",				V12dbeXtra::m_mCloneDatabase,		 1, 1,	700 },
	{ "mCreateField",				V12dbeXtra::m_mCreateField,		 -1, 0,	700 },
	{ "mCreateFullIndex",				V12dbeXtra::m_mCreateFullIndex,		 -1, 0,	700 },
	{ "mCreateIndex",				V12dbeXtra::m_mCreateIndex,		 -1, 0,	700 },
	{ "mCreateTable",				V12dbeXtra::m_mCreateTable,		 1, 1,	700 },
	{ "mCustom",				V12dbeXtra::m_mCustom,		 -1, 0,	700 },
	{ "mDeleteTable",				V12dbeXtra::m_mDeleteTable,		 1, 1,	700 },
	{ "mDumpStructure",				V12dbeXtra::m_mDumpStructure,		 -1, 0,	700 },
	{ "mEditDBStructure",				V12dbeXtra::m_mEditDBStructure,		 0, 0,	700 },
	{ "mGetPropertyNames",				V12dbeXtra::m_mGetPropertyNames,		 -1, 0,	700 },
	{ "mGetProperty",				V12dbeXtra::m_mGetProperty,		 1, 1,	700 },
	{ "mGetRef",				V12dbeXtra::m_mGetRef,		 0, 0,	700 },
	{ "mPackDatabase",				V12dbeXtra::m_mPackDatabase,		 1, 1,	700 },
	{ "mReadDBStructure",				V12dbeXtra::m_mReadDBStructure,		 -1, 0,	700 },
	{ "mRenameField",				V12dbeXtra::m_mRenameField,		 3, 3,	700 },
	{ "mSetPassword",				V12dbeXtra::m_mSetPassword,		 2, 2,	700 },
	{ "mSetProperty",				V12dbeXtra::m_mSetProperty,		 2, 2,	700 },
	{ "mUpdateDBStructure",				V12dbeXtra::m_mUpdateDBStructure,		 0, 0,	700 },
	{ "mError",				V12dbeXtra::m_mError,		 -1, 0,	700 },
	{ "mFixDatabase",				V12dbeXtra::m_mFixDatabase,		 2, 2,	700 },
	{ "mStatus",				V12dbeXtra::m_mStatus,		 0, 0,	700 },
	{ "mXtraVersion",				V12dbeXtra::m_mXtraVersion,		 0, 0,	700 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "V12Error", V12dbeXtra::m_V12Error, -1, 0, 700, HBLTIN },
	{ "V12ErrorReset", V12dbeXtra::m_V12ErrorReset, 0, 0, 700, HBLTIN },
	{ "V12Status", V12dbeXtra::m_V12Status, 0, 0, 700, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

/*
 * The V12 database file
 *
 * A V12 file is a set of B-trees in 1024-byte pages. Page n lives at file
 * offset (n + 1) * 1024. Each page starts with a 26-byte big-endian header:
 *
 *   u16 tree, u16 level (0 = leaf, 0xFFFF = overflow segment),
 *   u32 next, u32 prev, u16 itemCount, u16, u16 usedBytes, u16, u16 segSize,
 *   u32 (owner record id in overflow segments)
 *
 * Tree 3 holds every record, in record id order along the chain of leaves.
 * A leaf item is a tag byte and a u16 length (including these 3 bytes).
 * Items with tag bit 0x01 start a record: u32 recordId, then the total
 * length (u32 if tag bit 0x80 is set, else u16), then data. Items without
 * it continue the record before them. If (tag & 4) && (tag & 3) != 3 the
 * item ends in a u32 page pointer to a chain of overflow segments that
 * continue the data (usedBytes at offset 26, chained through next).
 *
 * A record payload is u16, u16 checksum, u16 0, u8 valueCount, then values.
 * System tables are records too, found through the leaves of their index
 * trees (tree 4 = fields, 5 = indexes, 6 = properties):
 *
 *   field:    table\0 field\0 u8 type, u32 size, u16 fieldNo
 *   index:    u16 tree, u16 flags, table\0 index\0 ...
 *   property: name\0 value\0
 *
 * Each table has an index "_UIDNdx" whose leaves list the record ids of its
 * rows. User values follow field order: strings as 0x10 + C string
 * (Windows-1252), integers as a big-endian s32.
 */

namespace {

const uint kV12PageSize = 1024;
const uint kV12HeaderSize = 26;
const uint16 kV12RecordTree = 3;
const uint16 kV12FieldTree = 4;
const uint16 kV12IndexTree = 5;
const uint16 kV12PropertyTree = 6;

struct V12Page {
	uint16 tree, level;
	uint32 next, prev;
	uint16 itemCount, used;
};

class V12Reader {
public:
	V12Reader(const byte *data, uint32 size) : _data(data), _size(size) {}

	uint pageCount() const { return _size / kV12PageSize - 1; }

	const byte *page(uint32 p) const { return _data + (p + 1) * kV12PageSize; }

	bool header(uint32 p, V12Page &h) const {
		if (p >= pageCount())
			return false;
		const byte *b = page(p);
		h.tree = READ_BE_UINT16(b);
		h.level = READ_BE_UINT16(b + 2);
		h.next = READ_BE_UINT32(b + 4);
		h.prev = READ_BE_UINT32(b + 8);
		h.itemCount = READ_BE_UINT16(b + 12);
		h.used = READ_BE_UINT16(b + 16);
		return true;
	}

	// Record ids in the leaves of an index tree
	Common::Array<uint32> leafIds(uint16 tree) const {
		Common::Array<uint32> ids;
		for (uint p = 0; p < pageCount(); p++) {
			V12Page h;
			header(p, h);
			if (h.tree != tree || h.level != 0)
				continue;
			const byte *b = page(p);
			uint o = kV12HeaderSize;
			for (uint i = 0; i < h.itemCount && o + 7 <= kV12PageSize; i++) {
				uint len = READ_BE_UINT16(b + o + 1);
				if (len < 7)
					break;
				ids.push_back(READ_BE_UINT32(b + o + 3));
				o += len;
			}
		}
		Common::sort(ids.begin(), ids.end());
		return ids;
	}

	bool records(Common::HashMap<uint32, Common::Array<byte> > &out) const {
		uint32 p = 0xffffffff;
		uint leaves = 0;
		for (uint i = 0; i < pageCount(); i++) {
			V12Page h;
			header(i, h);
			if (h.tree == kV12RecordTree && h.level == 0) {
				leaves++;
				if (h.prev == 0xffffffff)
					p = i;
			}
		}

		bool inRecord = false;
		uint32 recId = 0, total = 0;
		Common::Array<byte> data;
		uint visited = 0;
		while (p != 0xffffffff && visited++ < leaves) {
			V12Page h;
			if (!header(p, h))
				return false;
			const byte *b = page(p);
			uint o = kV12HeaderSize;
			for (uint i = 0; i < h.itemCount; i++) {
				if (o + 3 > kV12PageSize)
					return false;
				byte tag = b[o];
				uint len = READ_BE_UINT16(b + o + 1);
				if (len < 3 || o + len > kV12PageSize)
					return false;
				const byte *it = b + o;
				const byte *start, *end = it + len;
				o += len;

				if (tag & 0x01) {
					if (inRecord)
						warning("V12: record %d is incomplete", recId);
					recId = READ_BE_UINT32(it + 3);
					if (tag & 0x80) {
						total = READ_BE_UINT32(it + 7);
						start = it + 11;
					} else {
						total = READ_BE_UINT16(it + 7);
						start = it + 9;
					}
					data.clear();
					inRecord = true;
				} else {
					if (!inRecord)
						continue;	// stale continuation
					start = it + 3;
				}
				uint32 ptr = 0xffffffff;
				if ((tag & 4) && (tag & 3) != 3) {
					end -= 4;
					ptr = READ_BE_UINT32(end);
				}
				if (start > end)
					return false;
				append(data, start, end - start);
				uint segments = 0;
				while (ptr != 0xffffffff) {
					V12Page sh;
					if (!header(ptr, sh) || sh.level != 0xffff || segments++ > pageCount())
						return false;
					uint32 off = (ptr + 1) * kV12PageSize + kV12HeaderSize;
					if (off + sh.used > _size)
						return false;
					append(data, _data + off, sh.used);
					ptr = sh.next;
				}
				if (data.size() >= total) {
					if (data.size() != total)
						warning("V12: record %d is %d bytes, expected %d", recId, data.size(), total);
					out[recId] = data;
					inRecord = false;
				}
			}
			p = h.next;
		}
		return true;
	}

private:
	static void append(Common::Array<byte> &a, const byte *p, uint32 n) {
		uint32 old = a.size();
		a.resize(old + n);
		if (n)
			memcpy(a.data() + old, p, n);
	}

	const byte *_data;
	uint32 _size;
};

Common::String readCString(const Common::Array<byte> &v, uint &o) {
	Common::String s;
	while (o < v.size() && v[o])
		s += (char)v[o++];
	o++;
	return s;
}

Common::String decodeV12String(const Common::String &s) {
	return Common::U32String(s, Common::kWindows1252).encode(Common::kUtf8);
}

struct V12State {
	Common::Array<Common::SharedPtr<V12Database> > databases;
	int error = 0;
	Common::String errorMessage;
};

V12State *g_v12 = nullptr;

V12State &v12State() {
	if (!g_v12)
		g_v12 = new V12State;
	return *g_v12;
}

} // End of anonymous namespace

int V12Table::findField(const Common::String &fieldName) const {
	for (uint i = 0; i < fields.size(); i++)
		if (fields[i].name.equalsIgnoreCase(fieldName))
			return i;
	return -1;
}

Common::Array<Datum> V12Table::blankRecord() const {
	Common::Array<Datum> rec;
	for (auto &f : fields) {
		if (f.type == kV12Integer)
			rec.push_back(Datum(0));
		else
			rec.push_back(Datum(Common::String()));
	}
	return rec;
}

bool V12Database::load(Common::SeekableReadStream &stream) {
	uint32 size = stream.size();
	if (size < 2 * kV12PageSize)
		return false;
	Common::Array<byte> buf(size);
	if (stream.read(buf.data(), size) != size)
		return false;

	V12Reader reader(buf.data(), size);
	Common::HashMap<uint32, Common::Array<byte> > recs;
	if (!reader.records(recs))
		return false;

	auto payload = [&](uint32 id) -> const Common::Array<byte> * {
		if (!recs.contains(id) || recs[id].size() < 7)
			return nullptr;
		return &recs[id];
	};

	// Fields, ordered by field number within each table
	struct FieldDef {
		uint16 no;
		V12Field field;
	};
	Common::HashMap<Common::String, Common::Array<FieldDef>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> fieldDefs;
	Common::Array<Common::String> tableOrder;
	for (uint32 id : reader.leafIds(kV12FieldTree)) {
		const Common::Array<byte> *v = payload(id);
		if (!v)
			continue;
		uint o = 7;
		Common::String tableName = readCString(*v, o);
		FieldDef def;
		def.field.name = readCString(*v, o);
		if (o + 7 > v->size())
			continue;
		def.field.type = (*v)[o];
		def.field.size = READ_BE_UINT32(v->data() + o + 1);
		def.no = READ_BE_UINT16(v->data() + o + 5);
		if (!fieldDefs.contains(tableName))
			tableOrder.push_back(tableName);
		fieldDefs[tableName].push_back(def);
	}

	// Rows: the leaves of each table's _UIDNdx index
	Common::HashMap<Common::String, Common::Array<uint32>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> rowIds;
	for (uint32 id : reader.leafIds(kV12IndexTree)) {
		const Common::Array<byte> *v = payload(id);
		if (!v || v->size() < 11)
			continue;
		uint16 tree = READ_BE_UINT16(v->data() + 7);
		uint o = 11;
		Common::String tableName = readCString(*v, o);
		Common::String indexName = readCString(*v, o);
		if (indexName == "_UIDNdx")
			rowIds[tableName] = reader.leafIds(tree);
	}

	for (uint32 id : reader.leafIds(kV12PropertyTree)) {
		const Common::Array<byte> *v = payload(id);
		if (!v)
			continue;
		uint o = 7;
		Common::String name = readCString(*v, o);
		properties[name] = readCString(*v, o);
	}

	for (auto &tableName : tableOrder) {
		Common::Array<FieldDef> &defs = fieldDefs[tableName];
		Common::sort(defs.begin(), defs.end(), [](const FieldDef &a, const FieldDef &b) { return a.no < b.no; });
		V12Table table;
		table.name = tableName;
		for (auto &d : defs)
			table.fields.push_back(d.field);

		if (rowIds.contains(tableName)) {
			for (uint32 id : rowIds[tableName]) {
				const Common::Array<byte> *v = payload(id);
				if (!v) {
					warning("V12: %s: record %d of table %s is missing", fileName.c_str(), id, tableName.c_str());
					continue;
				}
				if ((*v)[6] != table.fields.size()) {
					warning("V12: %s: record %d of table %s has %d values, expected %d", fileName.c_str(), id, tableName.c_str(), (*v)[6], table.fields.size());
					continue;
				}
				Common::Array<Datum> rec;
				uint o = 7;
				for (auto &f : table.fields) {
					if (f.type == kV12Integer) {
						if (o + 4 > v->size())
							break;
						rec.push_back(Datum((int)(int32)READ_BE_UINT32(v->data() + o)));
						o += 4;
					} else if (f.type == kV12String && o < v->size() && (*v)[o] == 0x10) {
						o++;
						rec.push_back(Datum(decodeV12String(readCString(*v, o))));
					} else {
						warning("V12: %s: cannot decode field %s (type %d) of record %d", fileName.c_str(), f.name.c_str(), f.type, id);
						break;
					}
				}
				if (rec.size() == table.fields.size())
					table.records.push_back(rec);
			}
		}
		debugC(3, kDebugXObj, "V12: %s: table %s, %d fields, %d records", fileName.c_str(), table.name.c_str(), table.fields.size(), table.records.size());
		tables.push_back(table);
	}
	return true;
}

V12Table *V12Database::findTable(const Common::String &tableName) {
	for (auto &t : tables)
		if (t.name.equalsIgnoreCase(tableName))
			return &t;
	return nullptr;
}

// Changes to databases opened for writing go to a savefile instead of
// the original file, in a simple format of our own.
Common::String V12Database::saveName() const {
	return savePrefix() + fileName + ".v12sav";
}

bool V12Database::loadChanges() {
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(saveName());
	if (!in)
		return false;
	if (in->readUint32BE() != MKTAG('V', '1', '2', 'S')) {
		delete in;
		return false;
	}
	uint32 tableCount = in->readUint32BE();
	for (uint32 t = 0; t < tableCount && !in->eos(); t++) {
		Common::String name = in->readPascalString();
		uint32 fieldCount = in->readUint32BE();
		uint32 recordCount = in->readUint32BE();
		V12Table *table = findTable(name);
		if (table && table->fields.size() != fieldCount)
			table = nullptr;
		if (table)
			table->records.clear();
		for (uint32 r = 0; r < recordCount && !in->eos(); r++) {
			Common::Array<Datum> rec;
			for (uint32 f = 0; f < fieldCount; f++) {
				if (in->readByte() == kV12Integer) {
					rec.push_back(Datum((int)in->readSint32BE()));
				} else {
					uint32 len = in->readUint32BE();
					Common::String s;
					for (uint32 i = 0; i < len; i++)
						s += (char)in->readByte();
					rec.push_back(Datum(s));
				}
			}
			if (table)
				table->records.push_back(rec);
		}
	}
	bool ok = !in->err();
	delete in;
	return ok;
}

void V12Database::saveChanges() {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(saveName(), false);
	if (!out) {
		warning("V12: cannot save %s", saveName().c_str());
		return;
	}
	out->writeUint32BE(MKTAG('V', '1', '2', 'S'));
	out->writeUint32BE(tables.size());
	for (auto &t : tables) {
		out->writeByte(t.name.size());
		out->writeString(t.name);
		out->writeUint32BE(t.fields.size());
		out->writeUint32BE(t.records.size());
		for (auto &rec : t.records) {
			for (uint f = 0; f < t.fields.size(); f++) {
				if (t.fields[f].type == kV12Integer) {
					out->writeByte(kV12Integer);
					out->writeSint32BE(rec[f].asInt());
				} else {
					Common::String s = rec[f].asString();
					out->writeByte(kV12String);
					out->writeUint32BE(s.size());
					out->writeString(s);
				}
			}
		}
	}
	out->finalize();
	delete out;
}

Common::SharedPtr<V12Database> V12dbeXtra::getDatabase(int ref) {
	if (ref < 1 || ref > (int)v12State().databases.size())
		return Common::SharedPtr<V12Database>();
	return v12State().databases[ref - 1];
}

void V12dbeXtra::setError(int code, const Common::String &message) {
	v12State().error = code;
	v12State().errorMessage = message;
	if (code)
		warning("V12: %s", message.c_str());
}

V12dbeXtraObject::V12dbeXtraObject(ObjectType ObjectType) :Object<V12dbeXtraObject>("V12dbe") {
	_objType = ObjectType;
}

bool V12dbeXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum V12dbeXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(V12dbeXtra::xlibName);
	warning("V12dbeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void V12dbeXtra::open(ObjectType type, const Common::Path &path) {
	V12dbeXtraObject::initMethods(xlibMethods);
	V12dbeXtraObject *xobj = new V12dbeXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);

	// The same file also holds the V12table Xtra
	V12tableXtra::open(type, path);
}

void V12dbeXtra::close(ObjectType type) {
	V12dbeXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
	V12tableXtra::close(type);
}

// new(xtra "V12dbe", fileName, mode, password)
// Returns the database object, or an error code that mError explains.
void V12dbeXtra::m_new(int nargs) {
	Common::String password = nargs >= 3 ? g_lingo->pop().asString() : Common::String();
	Common::String mode = nargs >= 2 ? g_lingo->pop().asString() : Common::String("ReadOnly");
	Common::String origPath = nargs >= 1 ? g_lingo->pop().asString() : Common::String();
	if (nargs > 3)
		g_lingo->dropStack(nargs - 3);

	V12dbeXtraObject *me = static_cast<V12dbeXtraObject *>(g_lingo->_state->me.u.obj);
	Common::SharedPtr<V12Database> db(new V12Database);
	Common::Path path = findPath(origPath);
	Common::File file;
	if (path.empty() || !file.open(path)) {
		setError(-1, Common::String::format("Database file \"%s\" not found", origPath.c_str()));
		g_lingo->push(Datum(-1));
		return;
	}
	db->fileName = path.baseName();
	if (!db->load(file)) {
		setError(-2, Common::String::format("\"%s\" is not a V12 database", origPath.c_str()));
		g_lingo->push(Datum(-2));
		return;
	}
	db->readWrite = mode.equalsIgnoreCase("ReadWrite") || mode.equalsIgnoreCase("Create");
	if (db->readWrite && db->loadChanges())
		debugC(2, kDebugXObj, "V12dbeXtra::m_new: loaded changes from %s", db->saveName().c_str());

	v12State().databases.push_back(db);
	me->_db = db;
	me->_ref = v12State().databases.size();
	setError(0, Common::String());
	debugC(2, kDebugXObj, "V12dbeXtra::m_new: opened %s (%s), %d tables, ref %d", origPath.c_str(), mode.c_str(), db->tables.size(), me->_ref);
	g_lingo->push(g_lingo->_state->me);
}

void V12dbeXtra::m_mGetRef(int nargs) {
	g_lingo->dropStack(nargs);
	V12dbeXtraObject *me = static_cast<V12dbeXtraObject *>(g_lingo->_state->me.u.obj);
	g_lingo->push(Datum(me->_ref));
}

// mDumpStructure(db {, "tableNames" | "fieldNames" ...})
void V12dbeXtra::m_mDumpStructure(int nargs) {
	Common::String what = nargs >= 1 ? g_lingo->peek(nargs - 1).asString() : Common::String();
	g_lingo->dropStack(nargs);
	V12dbeXtraObject *me = static_cast<V12dbeXtraObject *>(g_lingo->_state->me.u.obj);
	Common::String res;
	if (me->_db) {
		for (auto &t : me->_db->tables) {
			if (what.equalsIgnoreCase("tableNames")) {
				res += t.name + "\r";
			} else {
				res += "Table " + t.name + "\r";
				for (auto &f : t.fields)
					res += Common::String::format("  %s %s\r", f.name.c_str(), f.type == kV12Integer ? "integer" : "string");
			}
		}
	}
	g_lingo->push(Datum(res));
}

void V12dbeXtra::m_mGetProperty(int nargs) {
	Common::String name = g_lingo->pop().asString();
	V12dbeXtraObject *me = static_cast<V12dbeXtraObject *>(g_lingo->_state->me.u.obj);
	if (me->_db && me->_db->properties.contains(name))
		g_lingo->push(Datum(me->_db->properties[name]));
	else
		g_lingo->push(Datum(Common::String()));
}

// mError(xtra "V12dbe" {, errorCode}): message of the last error
void V12dbeXtra::m_mError(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(v12State().errorMessage));
}

void V12dbeXtra::m_mStatus(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(v12State().error));
}

void V12dbeXtra::m_V12Error(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(v12State().errorMessage));
}

void V12dbeXtra::m_V12ErrorReset(int nargs) {
	g_lingo->dropStack(nargs);
	setError(0, Common::String());
	g_lingo->push(Datum());
}

void V12dbeXtra::m_V12Status(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(v12State().error));
}

XOBJSTUB(V12dbeXtra::m_mBuild, 0)
XOBJSTUB(V12dbeXtra::m_mCloneDatabase, 0)
XOBJSTUB(V12dbeXtra::m_mCreateField, 0)
XOBJSTUB(V12dbeXtra::m_mCreateFullIndex, 0)
XOBJSTUB(V12dbeXtra::m_mCreateIndex, 0)
XOBJSTUB(V12dbeXtra::m_mCreateTable, 0)
XOBJSTUB(V12dbeXtra::m_mCustom, 0)
XOBJSTUB(V12dbeXtra::m_mDeleteTable, 0)
XOBJSTUB(V12dbeXtra::m_mEditDBStructure, 0)
XOBJSTUB(V12dbeXtra::m_mFixDatabase, 0)
XOBJSTUB(V12dbeXtra::m_mGetPropertyNames, 0)
XOBJSTUB(V12dbeXtra::m_mPackDatabase, 0)
XOBJSTUB(V12dbeXtra::m_mReadDBStructure, 0)
XOBJSTUB(V12dbeXtra::m_mRenameField, 0)
XOBJSTUB(V12dbeXtra::m_mSetPassword, 0)
XOBJSTUB(V12dbeXtra::m_mSetProperty, 0)
XOBJSTUB(V12dbeXtra::m_mUpdateDBStructure, 0)
XOBJSTUB(V12dbeXtra::m_mXtraVersion, "3.0.0")

}
