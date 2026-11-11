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

#include "common/macresman.h"
#include "common/system.h"

#include "director/director.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/m/mapnavigatorxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- MapNav External Factory. 16/03/95 - AAF - New
MapNav
IS mNew, fileName -- read map given file name
X mDispose --  dispose of map
I mGetFirstNode -- return index of first node
X mResetHidden --  reset hidden flags to initial values
SI mGetNodeName, node --  given index, return node name
IS mGetNodeIndex, name -- given name, return node index
II mGetBackgroundPicture, node --  return picture number for node
II mGetHotSpotCount, node --   return no. of hot spots for node
XIII mSetHidden, node, hotspot, flag -- set/clear hidden flag
III mGetHidden, node, hotspot --  set/clear hidden flag
IIIII mPointInside, node, hotspot, x, y -- return true if x,y inside hot spot
SII mGetHotSpotRect, node, hotspot -- return list of l,t,r,b for hot spot
III mGetHotSpotCursor, node, hotspot -- return cursor number for node
SII mGetEvaluationFcn, node, hotspot -- return function name to call to get condition number
IIII mGetDestinationNode, node, hotspot, condition --  return destination node for hot spot
IIII mGetInstructionCount, node, hotspot, condition --  return number of instructions
IIIII mGetInstructionType, node, hotspot, condition, index -- return type of instruction 'Q' or 'L'
SIIII mGetInstruction, node, hotspot, condition, index --  return ith instruction

-- Navigator, Navigation XObject - aep 95.03.28
IS mNew, fileName -- read map given file name
X mDispose --  dispose of map
I mGetFirstNode -- return index of first node
X mResetHidden --  reset hidden flags to initial values
SI mGetNodeName, node --  given index, return node name
IS mGetNodeIndex, name -- given name, return node index
II mGetBackgroundPicture, node --  return picture number for node
II mGetHotSpotCount, node --   return no. of hot spots for node
XIII mSetHidden, node, hotspot, flag -- set/clear hidden flag
III mGetHidden, node, hotspot --  set/clear hidden flag
IIIII mPointInside, node, hotspot, h, v -- return true if h,v inside hot spot
SII mGetHotSpotRect, node, hotspot -- return list of l,t,r,b for hot spot
III mGetHotSpotCursor, node, hotspot -- return cursor number for node
SII mGetEvaluationFcn, node, hotspot -- return function name to call to get condition no.
IIII mGetDestinationNode, node, hotspot, condition --  return destination node for hot spot
IIII mGetInstructionCount, node, hotspot, condition --  return number of instructions
IIIII mGetInstructionType, node, hotspot, condition, i -- return type of instruction 'Q' or 'L'
SIIII mGetInstruction, node, hotspot, condition, i --  return ith instruction
 */

namespace Director {

const char *const MapNavigatorXObj::xlibNames[] = {
	"MapNav",			// Jewels of the Oracle - Win
	"MapNavigator",		// Jewels of the Oracle - Mac
	nullptr
};
const XlibFileDesc MapNavigatorXObj::fileNames[] = {
	{ "MAPNAV",				nullptr }, // Jewels of the Oracle - Win
	{ "MapNavigator",		nullptr }, // Jewels of the Oracle - Mac (XCOD resource name)
	{ "MapNavigator.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,				nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					MapNavigatorXObj::m_new,		 		 1, 1,	400 },
	{ "dispose",				MapNavigatorXObj::m_dispose,		 	 0, 0,	400 },
	{ "getFirstNode",			MapNavigatorXObj::m_getFirstNode,		 0, 0,	400 },
	{ "resetHidden",			MapNavigatorXObj::m_resetHidden,		 0, 0,	400 },
	{ "getNodeName",			MapNavigatorXObj::m_getNodeName,		 1, 1,	400 },
	{ "getNodeIndex",			MapNavigatorXObj::m_getNodeIndex,		 1, 1,	400 },
	{ "getBackgroundPicture",	MapNavigatorXObj::m_getBackgroundPicture,1, 1,	400 },
	{ "getHotSpotCount",		MapNavigatorXObj::m_getHotSpotCount,	 1, 1,	400 },
	{ "setHidden",				MapNavigatorXObj::m_setHidden,			 3, 3,	400 },
	{ "getHidden",				MapNavigatorXObj::m_getHidden,			 2, 2,	400 },
	{ "pointInside",			MapNavigatorXObj::m_pointInside,		 4, 4,	400 },
	{ "getHotSpotRect",			MapNavigatorXObj::m_getHotSpotRect,		 2, 2,	400 },
	{ "getHotSpotCursor",		MapNavigatorXObj::m_getHotSpotCursor,	 2, 2,	400 },
	{ "getEvaluationFcn",		MapNavigatorXObj::m_getEvaluationFcn,	 2, 2,	400 },
	{ "getDestinationNode",		MapNavigatorXObj::m_getDestinationNode,	 3, 3,	400 },
	{ "getInstructionCount",	MapNavigatorXObj::m_getInstructionCount, 3, 3,	400 },
	{ "getInstructionType",		MapNavigatorXObj::m_getInstructionType,	 4, 4,	400 },
	{ "getInstruction",			MapNavigatorXObj::m_getInstruction,		 4, 4,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MapNavigatorXObject::MapNavigatorXObject(ObjectType ObjectType) :Object<MapNavigatorXObject>("MapNav") {
	_objType = ObjectType;
}

void MapNavigatorXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MapNavigatorXObject::initMethods(xlibMethods);
		MapNavigatorXObject *xobj = new MapNavigatorXObject(kXObj);
		for (uint i = 0; xlibNames[i]; i++)
			g_lingo->exposeXObject(xlibNames[i], xobj);
	}
}

void MapNavigatorXObj::close(ObjectType type) {
	if (type == kXObj) {
		MapNavigatorXObject::cleanupMethods();
		for (uint i = 0; xlibNames[i]; i++)
			g_lingo->_globalvars[xlibNames[i]] = Datum();
	}
}

/*
 * Map file format (big-endian, also used by the Windows version):
 *
 *   uint16 nodeCount
 *   uint16 hotSpotCount        -- total over all nodes
 *   uint16 firstNode
 *   nodeCount x {
 *     uint16 backgroundPicture -- cast member number
 *     uint16 hotSpotCount
 *     uint32 offset            -- of the node's first hot spot
 *     pstring name             -- padded to an even length
 *   }
 *
 * Each hot spot:
 *   uint16 size                -- of the whole hot spot record
 *   Rect rect                  -- top, left, bottom, right
 *   uint16 cursor
 *   byte hidden, byte pad
 *   uint16 conditionCount
 *   pstring evaluationFcn      -- padded to an even length
 *   conditionCount x {
 *     uint16 size              -- of the whole condition record
 *     uint16 destinationNode
 *     uint16 condition         -- the value the evaluation function returns
 *     uint16 instructionCount
 *     instructionCount x {
 *       char type              -- 'Q' or 'L'
 *       uint16 length, string  -- padded to an even length
 *     }
 *   }
 *
 * Nodes are numbered from 0 in the file, but from 1 in the Lingo interface.
 * Hot spots and instructions are numbered from 1, while conditions are looked
 * up by their condition value.
 */

static Common::String readPaddedString(Common::SeekableReadStream *stream, bool wordLength) {
	uint len = wordLength ? stream->readUint16BE() : stream->readByte();
	Common::String str = stream->readString(0, len);
	if (stream->pos() & 1)
		stream->skip(1);
	return str;
}

bool MapNavigatorXObject::loadMap(Common::SeekableReadStream *stream) {
	_nodes.clear();

	uint16 nodeCount = stream->readUint16BE();
	stream->readUint16BE(); // total hot spot count
	_firstNode = stream->readUint16BE();

	Common::Array<uint32> offsets;
	_nodes.resize(nodeCount);
	offsets.resize(nodeCount);
	for (uint i = 0; i < nodeCount; i++) {
		Node &node = _nodes[i];
		node.backgroundPicture = stream->readSint16BE();
		node.hotSpots.resize(stream->readUint16BE());
		offsets[i] = stream->readUint32BE();
		node.name = readPaddedString(stream, false);
	}

	for (uint i = 0; i < nodeCount; i++) {
		Node &node = _nodes[i];
		uint32 pos = offsets[i];
		for (auto &hotSpot : node.hotSpots) {
			stream->seek(pos);
			uint16 size = stream->readUint16BE();
			hotSpot.rect.top = stream->readSint16BE();
			hotSpot.rect.left = stream->readSint16BE();
			hotSpot.rect.bottom = stream->readSint16BE();
			hotSpot.rect.right = stream->readSint16BE();
			hotSpot.cursor = stream->readSint16BE();
			hotSpot.initialHidden = hotSpot.hidden = stream->readByte() != 0;
			stream->skip(1);
			hotSpot.conditions.resize(stream->readUint16BE());
			hotSpot.evaluationFcn = readPaddedString(stream, false);

			for (auto &condition : hotSpot.conditions) {
				uint32 condPos = stream->pos();
				uint16 condSize = stream->readUint16BE();
				condition.destinationNode = stream->readSint16BE();
				condition.number = stream->readUint16BE();
				condition.instructions.resize(stream->readUint16BE());
				for (auto &instruction : condition.instructions) {
					instruction.type = stream->readByte();
					instruction.text = readPaddedString(stream, true);
				}
				stream->seek(condPos + condSize);
			}
			pos += size;
		}
	}

	if (stream->err()) {
		_nodes.clear();
		return false;
	}
	return true;
}

MapNavigatorXObject::Node *MapNavigatorXObject::getNode(int index) {
	if (index < 1 || index > (int)_nodes.size()) {
		warning("MapNavigatorXObject: Invalid node %d", index);
		return nullptr;
	}
	return &_nodes[index - 1];
}

MapNavigatorXObject::HotSpot *MapNavigatorXObject::getHotSpot(int node, int hotSpot) {
	Node *n = getNode(node);
	if (!n)
		return nullptr;
	if (hotSpot < 1 || hotSpot > (int)n->hotSpots.size()) {
		warning("MapNavigatorXObject: Invalid hot spot %d for node %d", hotSpot, node);
		return nullptr;
	}
	return &n->hotSpots[hotSpot - 1];
}

// The original keeps the hidden flags in one array for all hot spots and
// does no bounds checking, so a hot spot past the end of a node refers to
// one of the following node. Jewels of the Oracle relies on this when
// saving the state of "W1OH3", although node W1O has only two hot spots.
MapNavigatorXObject::HotSpot *MapNavigatorXObject::getHiddenHotSpot(int node, int hotSpot) {
	if (!getNode(node) || hotSpot < 1) {
		warning("MapNavigatorXObject: Invalid hot spot %d for node %d", hotSpot, node);
		return nullptr;
	}
	uint n = node - 1;
	uint index = hotSpot - 1;
	while (index >= _nodes[n].hotSpots.size()) {
		index -= _nodes[n].hotSpots.size();
		if (++n >= _nodes.size()) {
			warning("MapNavigatorXObject: Invalid hot spot %d for node %d", hotSpot, node);
			return nullptr;
		}
	}
	if (n != (uint)node - 1)
		debugC(5, kDebugXObj, "MapNavigatorXObject: Hot spot %d of node %d is hot spot %d of node %d", hotSpot, node, index + 1, n + 1);
	return &_nodes[n].hotSpots[index];
}

MapNavigatorXObject::Condition *MapNavigatorXObject::getCondition(int node, int hotSpot, int condition) {
	HotSpot *h = getHotSpot(node, hotSpot);
	if (!h)
		return nullptr;
	for (auto &c : h->conditions) {
		if (c.number == condition)
			return &c;
	}
	warning("MapNavigatorXObject: Invalid condition %d for node %d, hot spot %d", condition, node, hotSpot);
	return nullptr;
}

MapNavigatorXObject::Instruction *MapNavigatorXObject::getInstruction(int node, int hotSpot, int condition, int index) {
	Condition *c = getCondition(node, hotSpot, condition);
	if (!c)
		return nullptr;
	if (index < 1 || index > (int)c->instructions.size()) {
		warning("MapNavigatorXObject: Invalid instruction %d for node %d, hot spot %d, condition %d", index, node, hotSpot, condition);
		return nullptr;
	}
	return &c->instructions[index - 1];
}

static MapNavigatorXObject *getMe() {
	return static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);
}

// The methods below take node, hot spot, condition and instruction
// arguments in that order, after which any others follow

static MapNavigatorXObject::HotSpot *popHotSpot(bool hidden = false) {
	int hotSpot = g_lingo->pop().asInt();
	int node = g_lingo->pop().asInt();
	MapNavigatorXObject *me = getMe();
	return hidden ? me->getHiddenHotSpot(node, hotSpot) : me->getHotSpot(node, hotSpot);
}

static MapNavigatorXObject::Condition *popCondition() {
	int condition = g_lingo->pop().asInt();
	int hotSpot = g_lingo->pop().asInt();
	int node = g_lingo->pop().asInt();
	return getMe()->getCondition(node, hotSpot, condition);
}

static MapNavigatorXObject::Instruction *popInstruction() {
	int index = g_lingo->pop().asInt();
	int condition = g_lingo->pop().asInt();
	int hotSpot = g_lingo->pop().asInt();
	int node = g_lingo->pop().asInt();
	return getMe()->getInstruction(node, hotSpot, condition, index);
}

void MapNavigatorXObj::m_new(int nargs) {
	Common::String fileName = g_lingo->pop().asString();
	MapNavigatorXObject *me = getMe();

	Common::SeekableReadStream *stream = Common::MacResManager::openFileOrDataFork(findPath(fileName));
	if (!stream) {
		warning("MapNavigatorXObj::m_new: Could not open map file '%s'", fileName.c_str());
	} else {
		if (!me->loadMap(stream))
			warning("MapNavigatorXObj::m_new: Could not read map file '%s'", fileName.c_str());
		else
			debugC(5, kDebugXObj, "MapNavigatorXObj::m_new: Loaded %d nodes from '%s'", me->_nodes.size(), fileName.c_str());
		delete stream;
	}

	g_lingo->push(g_lingo->_state->me);
}

void MapNavigatorXObj::m_dispose(int nargs) {
	getMe()->_nodes.clear();
}

void MapNavigatorXObj::m_getFirstNode(int nargs) {
	g_lingo->push(Datum(getMe()->_firstNode + 1));
}

void MapNavigatorXObj::m_resetHidden(int nargs) {
	for (auto &node : getMe()->_nodes)
		for (auto &hotSpot : node.hotSpots)
			hotSpot.hidden = hotSpot.initialHidden;
}

void MapNavigatorXObj::m_getNodeName(int nargs) {
	int index = g_lingo->pop().asInt();
	MapNavigatorXObject::Node *node = getMe()->getNode(index);
	g_lingo->push(Datum(node ? node->name : Common::String()));
}

void MapNavigatorXObj::m_getNodeIndex(int nargs) {
	Common::String name = g_lingo->pop().asString();
	MapNavigatorXObject *me = getMe();

	// The original does a case-sensitive binary search, relying on the
	// nodes being sorted by name. It compares unsigned bytes and then
	// lengths, like compareTo.
	int lo = 0, hi = (int)me->_nodes.size() - 1;
	while (lo <= hi) {
		int mid = (lo + hi) / 2;
		int cmp = name.compareTo(me->_nodes[mid].name);
		if (cmp == 0) {
			g_lingo->push(Datum(mid + 1));
			return;
		}
		if (cmp < 0)
			hi = mid - 1;
		else
			lo = mid + 1;
	}
	warning("MapNavigatorXObj::m_getNodeIndex: Node '%s' not found", name.c_str());
	g_lingo->push(Datum(0));
}

void MapNavigatorXObj::m_getBackgroundPicture(int nargs) {
	int index = g_lingo->pop().asInt();
	MapNavigatorXObject::Node *node = getMe()->getNode(index);
	g_lingo->push(Datum(node ? node->backgroundPicture : 0));
}

void MapNavigatorXObj::m_getHotSpotCount(int nargs) {
	int index = g_lingo->pop().asInt();
	MapNavigatorXObject::Node *node = getMe()->getNode(index);
	g_lingo->push(Datum(node ? (int)node->hotSpots.size() : 0));
}

void MapNavigatorXObj::m_setHidden(int nargs) {
	int flag = g_lingo->pop().asInt();
	MapNavigatorXObject::HotSpot *h = popHotSpot(true);
	if (h)
		h->hidden = flag != 0;
}

void MapNavigatorXObj::m_getHidden(int nargs) {
	MapNavigatorXObject::HotSpot *h = popHotSpot(true);
	g_lingo->push(Datum(h && h->hidden ? 1 : 0));
}

void MapNavigatorXObj::m_pointInside(int nargs) {
	int v = g_lingo->pop().asInt();
	int h = g_lingo->pop().asInt();
	MapNavigatorXObject::HotSpot *hs = popHotSpot();
	g_lingo->push(Datum(hs && hs->rect.contains(h, v) ? 1 : 0));
}

void MapNavigatorXObj::m_getHotSpotRect(int nargs) {
	MapNavigatorXObject::HotSpot *h = popHotSpot();
	if (!h) {
		g_lingo->push(Datum(Common::String()));
		return;
	}
	const Common::Rect &r = h->rect;
	g_lingo->push(Datum(Common::String::format("%d,%d,%d,%d", r.left, r.top, r.right, r.bottom)));
}

void MapNavigatorXObj::m_getHotSpotCursor(int nargs) {
	MapNavigatorXObject::HotSpot *h = popHotSpot();
	g_lingo->push(Datum(h ? h->cursor : 0));
}

void MapNavigatorXObj::m_getEvaluationFcn(int nargs) {
	MapNavigatorXObject::HotSpot *h = popHotSpot();
	g_lingo->push(Datum(h ? h->evaluationFcn : Common::String()));
}

void MapNavigatorXObj::m_getDestinationNode(int nargs) {
	MapNavigatorXObject::Condition *c = popCondition();
	g_lingo->push(Datum(c ? c->destinationNode + 1 : 0));
}

void MapNavigatorXObj::m_getInstructionCount(int nargs) {
	MapNavigatorXObject::Condition *c = popCondition();
	g_lingo->push(Datum(c ? (int)c->instructions.size() : 0));
}

void MapNavigatorXObj::m_getInstructionType(int nargs) {
	MapNavigatorXObject::Instruction *i = popInstruction();
	g_lingo->push(Datum(i ? (int)(byte)i->type : 0));
}

void MapNavigatorXObj::m_getInstruction(int nargs) {
	MapNavigatorXObject::Instruction *i = popInstruction();
	g_lingo->push(Datum(i ? i->text : Common::String()));
}

}
