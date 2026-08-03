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

#ifndef EDNA_PATHFINDER_H
#define EDNA_PATHFINDER_H

#include "edna/util.h"

namespace Edna {

class PathFinder {
public:
    inline Common::Span<const byte> map() const {
        return { _map, kScreenWidth * kScreenHeight };
    }

    void loadArea(const char *fileName);
    bool findPath(Common::Point from, Common::Point to, Common::Array<Common::Point> &waypoints);
	Common::Point nearestWalkablePoint(Common::Point pos) const;
private:
    bool dijkstraDistances(Common::Point from, Common::Point to);
    void dijkstraPath(Common::Point from, Common::Point to, Common::Array<Common::Point> &waypoints);
    void reduceWaypoints(Common::Array<Common::Point> &waypoints) const;
    bool isWalkable(Common::Point pos) const;
    uint32 &distance(Common::Point pos);

	// a flat queue using a ring buffer
	class PointQueue {
	public:
		~PointQueue();
		void enqueue(Common::Point p);
		bool tryDequeue(Common::Point &p);
		void clear();
		inline uint32 capacity() const { return _capacity; } ///< for debugging

	private:
		uint32 _first = 0, _count = 0, _capacity = 0;
		Common::Point *_data = nullptr;
	};

	// these large, static data blocks is why we use a single instance of PathFinder for the engine
    byte _map[kScreenWidth * kScreenHeight]; ///< unfortunately this is column-major
    uint32 _distance[kScreenWidth * kScreenHeight];
	Common::Rect _bounds; ///< we reduce the screen bounds to speed up nearestWalkablePoint
    PointQueue _queue; ///< only used locally in dijkstraDistances but we keep it to reduce allocations
};

}

#endif // EDNA_PATHFINDER_H
