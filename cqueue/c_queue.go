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

// Thin wrapper around the string_queue C++ class so
// we only need to deal with unrecognized variables in this file.
type CStringQueue struct {
	queue SwigcptrString_queue
}

func NewCStringQueue(buffer_size int) *CStringQueue {
	if val, ok := NewString_queue(buffer_size).(SwigcptrString_queue); ok {
		return &CStringQueue{
			queue: val,
		}
	}
	return nil
}

func (q *CStringQueue) GetPtr() uint64 {
	return q.queue.Get_ptr()
}

func (q *CStringQueue) Pop() string {
	return q.queue.Pop()
}

func (q *CStringQueue) BlockingPop() string {
	return q.queue.Blocking_pop()
}

func (q *CStringQueue) Close() {
	q.queue.Close()
}

func (q *CStringQueue) Closed() bool {
	return q.queue.Closed()
}

func (q *CStringQueue) Push(str string) {
	q.queue.Push(str)
}

func (q *CStringQueue) Delete() {
	DeleteString_queue(q.queue)
}

func CStringQueueFromPtr(ptr uint64) *CStringQueue {
	if val, ok := String_queueQueue_from_pointer(ptr).(SwigcptrString_queue); ok {
		return &CStringQueue{
			queue: val,
		}
	}
	return nil
}
