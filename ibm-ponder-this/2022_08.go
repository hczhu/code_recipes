package main

import (
	"fmt"
)

func CountDanceSequence(pn int) uint64 {
	var M uint64 = 3141592653
	const N int = 7

	// f(0, 0): the dancer is in the middle.
	// f(1, 1): two dancers have danced one unit on two sides.
	// f(1, 2): ...
	// f(1, 3): ...
	// f(2, 2): ...
	// f(2, 3): ...
	// f(3, 3): ...
	var ma = [N][N]uint64{
		// f(0, 0) -> 3 * f(0, 0) + 6 * f(1, 1)
		{3, 6},
		// f(1, 1) -> 2 * f(0, 0) + 4 * f(1, 2) + f(2, 2)
		{2, 0, 4, 0, 1},
		// f(1, 2) = 2 * f(0, 0) + 2 * f(1, 2) + 2 * f(1, 3) + f(2, 3)
		{2, 0, 2, 2, 0, 1},
		// f(1, 3) = 2 * f(0, 0) + 2 * f(1, 2)
		{2, 0, 2},
		// f(2, 2) = 2 * f(0, 0) + 4 * f(1, 3) + f(3, 3)
		{2, 0, 0, 4, 0, 0, 1},
		// f(2, 3) = 2 * f(0, 0) + 2 * f(1, 3)
		{2, 0, 0, 2},
		// f(3, 3) = 2 * f(0, 0)
		{2},
	}
	var pma [257][N][N]uint64
	for i := 0; i < N; i++ {
		for j := 0; j < N; j++ {
			pma[0][j][i] = ma[i][j]
		}
	}
	ma_mul := func(a, b [N][N]uint64) [N][N]uint64 {
		var res [N][N]uint64
		for i := 0; i < N; i++ {
			for j := 0; j < N; j++ {
				res[i][j] = 0
				for k := 0; k < N; k++ {
					res[i][j] += a[i][k] * b[k][j]
					res[i][j] %= M
				}
			}
		}
		return res
	}
	for i := 1; i <= 256; i++ {
		pma[i] = ma_mul(pma[i-1], pma[i-1])
	}
	var ans [N][N]uint64
	for i := 0; i < N; i++ {
		ans[i][i] = 1
	}
	for i := 0; i < pn; i++ {
		ans = ma_mul(ans, pma[i])
	}
	// fmt.Printf("%d\n", ans)
	// n = 1 distribution {
	//   4, 12,
	// }
	s := uint64(0)
	for i := 0; i < N; i++ {
		s += 4*ans[i][0] + 12*ans[i][1]
	}
	return s % M
}

func main() {
	testOne := func(pn int, expect uint64) {
		if CountDanceSequence(pn) != expect {
			fmt.Printf("f(%d) != %d\n", pn, expect)
			panic("Error")
		}
	}
	testOne(0, 16)
	testOne(3, 17342172)
	testOne(7, 2484449895)
	fmt.Printf("%d\n", CountDanceSequence(24))
	fmt.Printf("%d\n", CountDanceSequence(256))
}
