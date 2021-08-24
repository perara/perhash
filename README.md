# perhash
Because why not use ALL the antipatterns?

This is a proof of concept for a horribly fast Map implementation. Why is it fast you may ask? Well, instead of doing hashing, we are assigning a integer to a corresponding string
The assigned integer are then used to lookup at a specific position in a vector.

Why is it any good?
1. You have no overhead after first invocation.

Why is current solution bad?
Well...
1. Uses a global (singleton) table to keep track of String => Integer.
2. It is not optimal when size of the vector varies. Due to the issues in 1., hashmaps will be bound to have the size ofthe largest instantiated map.


So where do we go from here?
1. Need help to find better approaches to solve locallity while ALSO keeping fast lookups. So essentially, we want to translate strings to integers 
2. Generally, remove all catches from the solution and publish as a viable solution to EXTREEEEEEMLY fast key-value lookup
