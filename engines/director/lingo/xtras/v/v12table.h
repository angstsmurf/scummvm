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

#ifndef DIRECTOR_LINGO_XTRAS_V_V12TABLE_H
#define DIRECTOR_LINGO_XTRAS_V_V12TABLE_H

namespace Director {

class V12tableXtraObject : public Object<V12tableXtraObject> {
public:
	V12tableXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;

	struct Criterion {
		Common::String conjunction;	// empty for the first one, else "AND" / "OR"
		int field;
		Common::String op;
		Datum value;
	};
	struct OrderKey {
		int field;
		bool descending;
	};

	V12Table *table();
	bool matches(const Common::Array<Datum> &record) const;
	void select();
	Common::Array<Datum> *currentRecord();

	Common::SharedPtr<V12Database> _db;
	int _tableIndex = -1;
	Common::Array<Criterion> _criteria;
	Common::Array<OrderKey> _order;
	Common::Array<uint> _selection;	// indices into table()->records
	int _pos = -1;	// 0-based index into _selection, -1 for none
	enum { kNotEditing, kEditing, kAdding } _editMode = kNotEditing;
	Common::Array<Datum> _buffer;
};

namespace V12tableXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_mAddRecord(int nargs);
void m_mAutoBinding(int nargs);
void m_mBindField(int nargs);
void m_mCustom(int nargs);
void m_mDataFormat(int nargs);
void m_mDeleteRecord(int nargs);
void m_mEditRecord(int nargs);
void m_mExportSelection(int nargs);
void m_mFind(int nargs);
void m_mGenerateMembers(int nargs);
void m_mGetField(int nargs);
void m_mGetMedia(int nargs);
void m_mGetPosition(int nargs);
void m_mGetProperty(int nargs);
void m_mGetSelection(int nargs);
void m_mGetUnique(int nargs);
void m_mGo(int nargs);
void m_mGoFirst(int nargs);
void m_mGoLast(int nargs);
void m_mGoNext(int nargs);
void m_mGoPrevious(int nargs);
void m_mImport(int nargs);
void m_mImportFile(int nargs);
void m_mNeedSelect(int nargs);
void m_mOrderBy(int nargs);
void m_mRefreshBoundFields(int nargs);
void m_mSelDelete(int nargs);
void m_mSelect(int nargs);
void m_mSelectAll(int nargs);
void m_mSelectCount(int nargs);
void m_mSetCriteria(int nargs);
void m_mSetField(int nargs);
void m_mSetIndex(int nargs);
void m_mSetMedia(int nargs);
void m_mSetProperty(int nargs);
void m_mUnbindField(int nargs);
void m_mUpdateBoundFields(int nargs);
void m_mUpdateRecord(int nargs);

} // End of namespace V12tableXtra

} // End of namespace Director

#endif
