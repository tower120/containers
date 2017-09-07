Super fast, thread-safe, unordered, lockless, deque like(!) array.  
Allow emplace/erase during iteration!  
Elements placed, mostly, in continious spcace.

Size : 3pts[default] / 2ptrs[optionally]

Emplace O(1)  
Erase O(1) [default] / O(n) [optionally] Elements destruction will be postponed, untill last iteration loop will be finished.