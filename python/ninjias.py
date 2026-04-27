from tkinter import W
import unittest

from doubly_linked_list import DoubleLinks

Node = DoubleLinks[int]

def ninjas_bruteforce(n: int) -> int|None:
    tail: Node = Node(0)
    p0: Node = tail
    for i in range(1, n):
        tail: Node = tail.insert(Node(i))  
    tail.next = p0
    p0.prev = tail 
    p: Node = p0
    while p is not None and p.next is not None and p.next != p:
        assert p.next is not None
        p.next.remove()
        p = p.next
    return p.get_data()

def ninjias(n: int) -> int:
    """ n ninjas are standing in a circle, each holding a sword.
        #0 kills #1, #2 kills #3, and so on. It keeps going around and around until one ninja is left standing.
        Which ninja is it?
    """
    if n <= 2:
        return 0
    if n % 2 == 0:
        return 2 * ninjias(n // 2)
    res = ninjias((n+1)//2)
    if res == 0:
        return n - 1
    return (res - 1) * 2

# Write unit test cases using unittest framework for ninjas_bruteforce function. The test cases should cover edge cases and typical cases.
class TestNinjasBruteforce(unittest.TestCase):
    def test_ninjas(self):
        self.assertEqual(ninjas_bruteforce(1), 0)
        self.assertEqual(ninjas_bruteforce(2), 0)
        self.assertEqual(ninjas_bruteforce(3), 2)
        self.assertEqual(ninjas_bruteforce(4), 0)
        self.assertEqual(ninjas_bruteforce(5), 2)
        self.assertEqual(ninjas_bruteforce(6), 4)
        self.assertEqual(ninjas_bruteforce(7), 6)
        self.assertEqual(ninjas_bruteforce(8), 0)
        self.assertEqual(ninjas_bruteforce(9), 2)
        self.assertEqual(ninjas_bruteforce(10), 4)
        self.assertEqual(ninjas_bruteforce(1000), 976)

        # Test the recursive function
        self.assertEqual(ninjias(1), 0)
        self.assertEqual(ninjias(2), 0)
        self.assertEqual(ninjias(3), 2)
        self.assertEqual(ninjias(4), 0)
        self.assertEqual(ninjias(5), 2)
        self.assertEqual(ninjias(6), 4)
        self.assertEqual(ninjias(7), 6)
        self.assertEqual(ninjias(8), 0)
        self.assertEqual(ninjias(9), 2)
        self.assertEqual(ninjias(10), 4)

        # Test that both functions agree on a larger input
        for n in range(100, 5000):
            self.assertEqual(ninjas_bruteforce(n), ninjias(n))

if __name__ == "__main__":
    unittest.main()