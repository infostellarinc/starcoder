/*
 * Starcoder - a server to read/write data from/to the stars, written in Go.
 * Copyright (C) 2018 InfoStellar, Inc.
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
 */

package cqueue

// Thin wrapper around the c_queue C++ class so
// we only need to deal with unrecognized variables in this file.
type CQueue struct {
	queue SwigcptrStringQueue
}

func NewCQueue(buffer_size int) *CQueue {
	if val, ok := NewStringQueue(buffer_size).(SwigcptrStringQueue); ok {
		return &CQueue{
			queue: val,
		}
	}
	return nil
}

func (q *CQueue) GetPtr() uint64 {
	return q.queue.Get_ptr()
}

func (q *CQueue) Pop() string {
	return q.queue.Pop()
}

func (q *CQueue) BlockPop() string {
	return q.queue.Block_pop()
}

func (q *CQueue) Wake() {
	q.queue.Wake()
}

func (q *CQueue) Close() {
	DeleteStringQueue(q.queue)
}
