type Value struct {
	ver int
	v   int
}

type SnapshotArray struct {
	sid int
	arr [][]Value
}

func Constructor(length int) SnapshotArray {
	return SnapshotArray{
		sid: 0,
		arr: make([][]Value, length),
	}
}

func (this *SnapshotArray) Set(index int, val int) {
	this.arr[index] = append(this.arr[index], Value{
		ver: this.sid,
		v:   val,
	})
}

func (this *SnapshotArray) Snap() int {
	res := this.sid
	this.sid++
	return res
}

func (this *SnapshotArray) Get(index int, snap_id int) int {
	arr := this.arr[index]
	if len(arr) == 0 || arr[0].ver > snap_id {
		return 0
	}
	l, h := 0, len(arr)-1
	// Invariant: arr[l].ver <= snap_id
	for l < h {
		// Invariant: l < m <= h
		m := (l + h + 1) / 2
		if arr[m].ver <= snap_id {
			l = m
		} else {
			h = m - 1
		}
	}
	return arr[l].v
}