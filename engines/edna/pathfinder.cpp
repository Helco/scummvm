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
#include "common/file.h"

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
    if (!file.open(fileName))
        error("Could not open walkable area: %s", fileName);
    if (file.size() != kWamTotalSize)
        error("Unexpected total size of walkable area: %s (%u instead of %u)", fileName, (uint)file.size(), (uint)kWamTotalSize);
    byte header[kWamHeaderSize];
    if (file.read(header, kWamHeaderSize) != kWamHeaderSize ||
        memcmp(kWamHeader, header, kWamHeaderSize) != 0)
        error("Invalid header in walkable area: %s", fileName);

    for (uint32 x = 0; x < kScreenWidth; x++) {
        if (x > 0 && !file.skip(kWamDelimiterSize))
            error("Could not skip walkable area delimiter: %s", fileName);
        if (file.read(_map + x * kScreenHeight, kScreenHeight) != kScreenHeight)
            error("Could not read walkable area column: %s", fileName);
    }
}

bool PathFinder::findPath(Point from, Point to, Array<Point> &waypoints) {
    waypoints.clear();
    if (from == to) {
        waypoints.emplace_back(to);
        return true;
    }

    from = nearestWalkablePoint(from);
    to = nearestWalkablePoint(to);
    if (from == kInvalidPoint ||
        to == kInvalidPoint ||
        !dijkstraDistances(from, to))
        return false;
    dijkstraPath(from, to, waypoints);
	uint beforeReduction = waypoints.size();
    reduceWaypoints(waypoints);
    reverse(waypoints.begin(), waypoints.end());

	debugC(2, kDebugGameplay, "Path (%d,%d) -> (%d,%d), distance=%u, before=%u, after=%u, queueCap=%u",
		from.x, from.y, to.x, to.y, distance(to), beforeReduction, waypoints.size(), _queue.capacity());
    return true;
}

Point PathFinder::nearestWalkablePoint(Point pos) const {
    // The original game used euclidian distance and just iterated over 
    // the entire 800x600 map to find the nearest point
    // Instead we use Chebyshev, iterate near-to-far and can thus return
    // as soon as we find some walkable point
    // If we find these inaccuracies in the game, we can switch then
    if (isWalkable(pos))
        return pos;
    pos.x = (int16)CLIP<int>(pos.x, 0, kScreenWidth - 1);
    pos.y = (int16)CLIP<int>(pos.y, 0, kScreenHeight - 1);
    const int maxRadius = MAX<int>(MAX(pos.x, pos.y), MAX(kScreenWidth - pos.x - 1, kScreenHeight - pos.y - 1));
    for (int r = 1; r <= maxRadius; r++) {
        // top/bottom edges
        int minX = MAX(0, pos.x - r);
        int maxX = MIN(kScreenWidth - 1, pos.x + r);
        for (int x = minX; x <= maxX; x++) {
            if (pos.y - r >= 0 && isWalkable(Point(x, pos.y - r)))
                return Point(x, pos.y - r);
            if (pos.y + r < kScreenHeight && isWalkable(Point(x, pos.y + r)))
                return Point(x, pos.y + r);
        }

        // left/right edges
        int minY = MAX(0, pos.y - r) + 1;
        int maxY = MIN(kScreenHeight - 1, pos.y + r) - 1;
        for (int y = minY; y <= maxY; y++) {
            if (pos.x - r >= 0 && isWalkable(Point(pos.x - r, y)))
                return Point(pos.x - r, y);
            if (pos.x + r < kScreenWidth && isWalkable(Point(pos.y + r, y)))
                return Point(pos.x + r, y);
        }
    }
    return kInvalidPoint;
}

struct Neighbors {
    static constexpr const Point kOffsets[] = {
        Point(-1, -1),
        Point(1, 0),
        Point(1, 0),
        Point(-2, 1),
        Point(2, 0),
        Point(-2, 1),
        Point(1, 0),
        Point(1, 0)
    };
    struct Iterator {
        Point _pos;
        int _step = 0;

        Iterator(Point center, int step) : _step(step) {
            _pos = center + kOffsets[0];
        }

        Iterator &operator++() {
            assert(_step < 9);
            do
            {
                _pos += kOffsets[++_step];
            } while (_step < 9 && !Rect(kScreenWidth, kScreenHeight).contains(_pos));
            return *this;
        }

        const Point &operator*() const {
            assert(_step < 9);
            return _pos;
        }

        bool operator==(const Iterator &o) const { return _step == o._step; }
        bool operator!=(const Iterator &o) const { return _step != o._step; }
    };

    Point _center;
    Neighbors(Point center) : _center(center) {}
    Iterator begin() const { return Iterator(_center, 0); }
    Iterator end() const { return Iterator(_center, 9); }
};

bool PathFinder::dijkstraDistances(Point from, Point to) {
    fill(_distance, _distance + kScreenWidth * kScreenHeight, UINT32_MAX);
    _queue.clear();
	_queue.enqueue(from);
    distance(from) = 0;
    Point cur;
    while (_queue.tryDequeue(cur)) {
        if (cur == to)
            return true;
        auto newDist = distance(cur) + 1;
        for (const auto &neighbor : Neighbors(cur)) {
            if (isWalkable(neighbor) && newDist < distance(neighbor)) {
                distance(neighbor) = newDist;
                _queue.enqueue(neighbor);
            }
        }
    }
	return false;
}

void PathFinder::dijkstraPath(Point from, Point to, Array<Point> &waypoints) {
    // we do not reserve the waypoints to distance(to) because we already reduce 
    // the waypoints quite a lot here, no need to always overallocate

    const auto lastDelta = [&]() {
        const uint n = waypoints.size();
        if (n < 2)
            return Point();
        return waypoints[n - 1] - waypoints[n - 2];
    };

    Point cur = to;
    while (cur != from) {
        if (lastDelta() == cur - waypoints.back())
            waypoints.back() = cur;
        else
            waypoints.emplace_back(cur);

        Point bestNeighbor;
        uint32 bestDistance = UINT32_MAX;
        for (const auto &neighbor : Neighbors(cur)) {
            if (distance(neighbor) < bestDistance) {
                bestDistance = distance(neighbor);
                bestNeighbor = neighbor;
            }
        }
        cur = bestNeighbor;
    }
}

void PathFinder::reduceWaypoints(Array<Point> &waypoints) const {
    // For reduction we find triples of points that lie approximately on the same line
    // for such triples we can remove the center point
    // the original game used vector math, I will use only integer operations as such
    // there will be more waypoints but as long as they are still few enough
    if (waypoints.size() < 3)
        return;

    for (uint32 i = waypoints.size() - 1; i >= 2; i--) {
        auto largeDelta = waypoints[i - 2] - waypoints[i - 0];
        auto smallDelta = waypoints[i - 1] - waypoints[i - 0];
        largeDelta = largeDelta / gcd(largeDelta.x, largeDelta.y);
        smallDelta = smallDelta / gcd(smallDelta.x, smallDelta.y);
        if (largeDelta != smallDelta)
            continue;
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
		if (_capacity == 0)
			_capacity = 512; // for walking across the map, TODO: check usual capacities for large walks
		Point *newData = new Point[_capacity *= 2];
		copy(_data + _first, _data + _capacity - _first, newData); // reorder during copy
		copy(_data, _data + _first, newData + _capacity - _first);
		delete[] _data;
		_first = 0;
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
