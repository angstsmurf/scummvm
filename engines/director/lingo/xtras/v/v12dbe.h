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

#ifndef DIRECTOR_LINGO_XTRAS_V_V12DBE_H
#define DIRECTOR_LINGO_XTRAS_V_V12DBE_H

#include "common/ptr.h"

namespace Director {

enum V12FieldType {
	kV12Integer = 4,
	kV12String = 16,
};

struct V12Field {
	Common::String name;
	byte type;
	uint32 size;
};

struct V12Table {
	Common::String name;
	Common::Array<V12Field> fields;
	// One Datum per field: INT for integer fields, STRING (UTF-8) otherwise
	Common::Array<Common::Array<Datum> > records;

	int findField(const Common::String &fieldName) const;
	Common::Array<Datum> blankRecord() const;
};

struct V12Database {
	Common::String fileName;	// for savefile naming and error messages
	bool readWrite = false;
	Common::Array<V12Table> tables;
	Common::StringMap properties;

	bool load(Common::SeekableReadStream &stream);
	V12Table *findTable(const Common::String &tableName);

	Common::String saveName() const;
	bool loadChanges();
	void saveChanges();
};

class V12dbeXtraObject : public Object<V12dbeXtraObject> {
public:
	V12dbeXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;

	Common::SharedPtr<V12Database> _db;
	int _ref = 0;
};

namespace V12dbeXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

Common::SharedPtr<V12Database> getDatabase(int ref);
void setError(int code, const Common::String &message);

void m_new(int nargs);
void m_mBuild(int nargs);
void m_mCloneDatabase(int nargs);
void m_mCreateField(int nargs);
void m_mCreateFullIndex(int nargs);
void m_mCreateIndex(int nargs);
void m_mCreateTable(int nargs);
void m_mCustom(int nargs);
void m_mDeleteTable(int nargs);
void m_mDumpStructure(int nargs);
void m_mEditDBStructure(int nargs);
void m_mError(int nargs);
void m_mFixDatabase(int nargs);
void m_mGetPropertyNames(int nargs);
void m_mGetProperty(int nargs);
void m_mGetRef(int nargs);
void m_mPackDatabase(int nargs);
void m_mReadDBStructure(int nargs);
void m_mRenameField(int nargs);
void m_mSetPassword(int nargs);
void m_mSetProperty(int nargs);
void m_mStatus(int nargs);
void m_mUpdateDBStructure(int nargs);
void m_V12Error(int nargs);
void m_V12ErrorReset(int nargs);
void m_V12Status(int nargs);
void m_mXtraVersion(int nargs);

} // End of namespace V12dbeXtra

} // End of namespace Director

#endif
