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

#include "edna/detection.h"
#include "edna/pathfinder.h"
#include "edna/util.h"

#include "common/algorithm.h"
#include "math/line2d.h"

using namespace Common;

namespace Edna {

// The WAM files are Java serialized objects with a single boolean[800][600]
// The structure is thus always the same and we can assert the total size
// Asserting the header at top should then suffice

static constexpr const byte kWamHeader[] = {
	0xac, 0xed, 0x00, 0x05, 0x75, 0x72, 0x00, 0x03, 
	0x5b, 0x5b, 0x5a, 0x63, 0x1f, 0x17, 0x21, 0xcb, 
	0x9a, 0x1c, 0x15, 0x02, 0x00, 0x00, 0x78, 0x70, 
	0x00, 0x00, 0x03, 0x20, 0x75, 0x72, 0x00, 0x02, 
	0x5b, 0x5a, 0x57, 0x8f, 0x20, 0x39, 0x14, 0xb8, 
	0x5d, 0xe2, 0x02, 0x00, 0x00, 0x78, 0x70, 0x00, 
	0x00, 0x02, 0x58
};
static constexpr const uint32 kWamHeaderSize = ARRAYSIZE(kWamHeader);
static constexpr const uint32 kWamDelimiterSize = 10;
static constexpr const uint32 kWamTotalSize =
    kWamHeaderSize +
    (kScreenWidth - 1) * kWamDelimiterSize +
    kScreenWidth * kScreenHeight;

void PathFinder::loadArea(const char *fileName) {
    assert(fileName != nullptr);
    File file;
    if (!file.open(fileName) && !file.open(Path(String("data/map/") + fileName)))
        error("Could not open walkable area: %s", fileName);
    if (file.size() != kWamTotalSize)
        error("Unexpected total size of walkable area: %s (%u instead of %u)", fileName, (uint)file.size(), (uint)kWamTotalSize);
    byte header[kWamHeaderSize];
    if (file.read(header, kWamHeaderSize) != kWamHeaderSize ||
        memcmp(kWamHeader, header, kWamHeaderSize) != 0)
        error("Invalid header in walkable area: %s", fileName);

	Point max(-1, -1), min(10000, 10000);
    for (int16 x = 0; x < kScreenWidth; x++) {
        if (x > 0 && !file.skip(kWamDelimiterSize))
            error("Could not skip walkable area delimiter: %s", fileName);
		byte *line = _map + x * kScreenHeight;
        if (file.read(line, kScreenHeight) != kScreenHeight)
            error("Could not read walkable area column: %s", fileName);

		for (int16 y = 0; y < kScreenHeight; y++) {
			if (!line[y])
				continue;
			max.x = MAX(max.x, x);
			max.y = MAX(max.y, y);
			min.x = MIN(min.x, x);
			min.y = MIN(min.y, y);
		}
    }
	max += Point(1, 1);
	_bounds = min.x < max.x && min.y < max.y ? Rect(min, max) : Rect();
}

bool PathFinder::findPath(Point fromOrig, Point toOrig, Array<Point> &waypoints) {
    waypoints.clear();
	if (fromOrig == toOrig || _bounds.isEmpty())
		return false;

    Point from = nearestWalkablePoint(fromOrig);
	Point to = nearestWalkablePoint(toOrig);
	if (debugChannelSet(0, kDebugPathFinder) && (from != fromOrig || to != toOrig))
		debug("Path corrected from (%d,%d)->(%d,%d) to (%d,%d)->(%d,%d)",
			fromOrig.x, fromOrig.y, toOrig.x, toOrig.y, from.x, from.y, to.x, to.y);

    if (from == kInvalidPoint ||
        to == kInvalidPoint ||
        !dijkstraDistances(from, to))
        return false;
    dijkstraPath(from, to, waypoints);
	uint beforeReduction = waypoints.size();
    reduceWaypoints(waypoints);

	debugC(kDebugPathFinder, "Path (%d,%d) -> (%d,%d), distance=%u, before=%u, after=%u, queueCap=%u",
		from.x, from.y, to.x, to.y, distance(to) >> 4, beforeReduction, waypoints.size(), _queue.capacity());
    return true;
}

Point PathFinder::nearestWalkablePoint(Point pos) const {
    // The original game used Euclidian distance and just iterated over 
    // the entire 800x600 map to find the nearest point
	// Instead we go in Chebyshev order and break as soon as there cannot
	// be any better solution.
	// This should have optimal precision but reduced runtime
    if (isWalkable(pos))
        return pos;
	const auto relPos = pos - _bounds.origin();
    const int maxRadius = MAX<int>(
		MAX(relPos.x, relPos.y),
		MAX(_bounds.width() - relPos.x - 1, _bounds.height() - relPos.y - 1));

	Point bestPoint;
	uint bestDistanceSqr = UINT_MAX;
	const auto evaluate = [&](Point p) {
		uint curDistanceSqr = p.sqrDist(pos);
		if (curDistanceSqr < bestDistanceSqr) {
			bestPoint = p;
			bestDistanceSqr = curDistanceSqr;
		}
	};

    for (int r = 1; r <= maxRadius; r++) {
        // top/bottom edges
        int minX = MAX((int)_bounds.left, pos.x - r);
        int maxX = MIN(_bounds.right - 1, pos.x + r);
        for (int x = minX; x <= maxX; x++) {
			if (pos.y - r >= _bounds.top && isWalkable(Point(x, pos.y - r)))
				evaluate(Point(x, pos.y - r));
            if (pos.y + r < _bounds.bottom && isWalkable(Point(x, pos.y + r)))
				evaluate(Point(x, pos.y + r));
        }

        // left/right edges
        int minY = MAX((int)_bounds.top, pos.y - r) + 1;
        int maxY = MIN(_bounds.bottom - 1, pos.y + r) - 1;
        for (int y = minY; y <= maxY; y++) {
			if (pos.x - r >= _bounds.left && isWalkable(Point(pos.x - r, y)))
				evaluate(Point(pos.x - r, y));
            if (pos.x + r < _bounds.right && isWalkable(Point(pos.x + r, y)))
				evaluate(Point(pos.x + r, y));
        }

		if ((uint)(r * r) >= bestDistanceSqr)
			return bestPoint;
    }

	warning("Could not find any walkable point (%d, %d), this should not have happened", pos.x, pos.y);
    return kInvalidPoint;
}

// A range-for-compatible neighborhood that already filters out-of-screen points

static constexpr const Point kNeighborOffsets[] = {
	// these offset list is biased 
	Point(0, -1), // top
	Point(1, 1), // right
	Point(-1, 1), // down
	Point(-1, -1), // left
	Point(0, -1), // top-left
	Point(2, 0), // top-right
	Point(0, 2), // bottom-right
	Point(-2, 0), // bottom-left
	Point(-1000, -1000), // end sentinel
};
struct Neighbors {
    struct Iterator {
        Point _pos;
        int _step = 0;

        Iterator(Point center, int step) : _step(step) {
            _pos = center;
			++*this;
        }

        Iterator &operator++() {
            do
            {
                _pos += kNeighborOffsets[_step++];
            } while (_step < 9 && !Rect(kScreenWidth, kScreenHeight).contains(_pos));
            return *this;
        }

        const Point &operator*() const {
            assert(_step > 0 && _step < 9);
            return _pos;
        }

        bool operator==(const Iterator &o) const { return _step == o._step; }
        bool operator!=(const Iterator &o) const { return _step != o._step; }
    };

    Point _center;
    Neighbors(Point center) : _center(center) {}
    Iterator begin() const { return Iterator(_center, 0); }
    Iterator end() const { return Iterator(_center, 8); }
};

bool PathFinder::dijkstraDistances(Point from, Point to) {
	// only reset distances we can reach
	Rect fillBounds = _bounds;
	fillBounds.grow(2);
	fillBounds.clip(Rect(0, 0, kScreenWidth, kScreenHeight));
	for (int16 x = fillBounds.left; x < fillBounds.right; x++) {
		uint32 *line = _distance + x * kScreenHeight;
		fill(line + fillBounds.top, line + fillBounds.bottom, UINT32_MAX);
	}

    _queue.clear();
	_queue.enqueue(from);
    distance(from) = 0;
    Point cur;
    while (_queue.tryDequeue(cur)) {
        if (cur == to)
            return true;
        auto oldDist = distance(cur);
		assert(oldDist > 0 || cur == from);
        for (const auto &neighbor : Neighbors(cur)) {
            if (isWalkable(neighbor) && distance(neighbor) == UINT32_MAX) {
				distance(neighbor) = oldDist + 1;
                _queue.enqueue(neighbor);
            }
        }
    }
	return false;
}

void PathFinder::dijkstraPath(Point from, Point to, Array<Point> &waypoints) {
    // we do not reserve the waypoints to distance(to) because we already reduce 
    // the waypoints quite a lot here, no need to always overallocate

    Point cur = to;
	Point lastDelta(1000, 1000), curDelta(-1000, -1000);
    while (cur != from) {
		if (lastDelta == curDelta)
			waypoints.back() = cur;
		else {
			waypoints.emplace_back(cur);
			lastDelta = curDelta;
		}

        Point bestNeighbor;
		uint32 bestDistance = distance(cur);
        for (const auto &neighbor : Neighbors(cur)) {
			if (!isWalkable(neighbor))
				continue;
			uint32 curDistance = distance(neighbor);
            if (curDistance < bestDistance) {
				bestDistance = curDistance;
                bestNeighbor = neighbor;
            }
        }
		assert(bestDistance < distance(cur));
		curDelta = bestNeighbor - cur;
        cur = bestNeighbor;
    }
}

static Math::Vector2d asVec(Point point) {
	return Math::Vector2d(point.x, point.y);
}

void PathFinder::reduceWaypoints(Array<Point> &waypoints) const {
    // For reduction we find triples of points that lie approximately on the same line
    // for such triples we can remove the center point
    if (waypoints.size() < 3)
        return;

    for (uint32 i = waypoints.size() - 1; i >= 2; i--) {
		auto edgeDir = asVec(waypoints[i - 2] - waypoints[i]).getNormalized();
		Math::Vector2d edgeNormal(-edgeDir.getY(), edgeDir.getX());
		float dist = fabsf(
			edgeNormal.dotProduct(asVec(waypoints[i - 1])) -
			edgeNormal.dotProduct(asVec(waypoints[i])));
		if (dist < 2.0f)
			waypoints.remove_at(i - 1);
    }
}

bool PathFinder::isWalkable(Point pos) const {
    return pos.x < 0 || pos.y < 0 || pos.x >= kScreenWidth || pos.y >= kScreenHeight
        ? false
        : _map[pos.x * kScreenHeight + pos.y] != 0;
}

uint32 &PathFinder::distance(Point pos) {
	assert(Rect(kScreenWidth, kScreenHeight).contains(pos));
	return _distance[pos.x * kScreenHeight + pos.y];
}

PathFinder::PointQueue::~PointQueue() {
	if (_data != nullptr) {
		delete[] _data;
		_data = nullptr;
	}
}

void PathFinder::PointQueue::enqueue(Point p) {
	if (_count == _capacity) {
		uint32 newCapacity = _capacity == 0 ? 512 : _capacity * 2;
		Point *newData = new Point[newCapacity];
		if (_data != nullptr) {
			copy(_data + _first, _data + _capacity, newData); // reorder during copy
			copy(_data, _data + _first, newData + _capacity - _first);
			delete[] _data;
		}
		_data = newData;
		_first = 0;
		_capacity = newCapacity;
	}

	_data[(_first + _count) % _capacity] = p;
	_count++;
}

bool PathFinder::PointQueue::tryDequeue(Point &p) {
	if (_count == 0)
		return false;
	p = _data[_first++];
	_first %= _capacity;
	_count--;
	return true;
}

void PathFinder::PointQueue::clear() {
	_first = _count = 0;
}

}
