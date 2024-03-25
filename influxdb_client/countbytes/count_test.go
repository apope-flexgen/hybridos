package countbytes

import "testing"

func TestSizeOfJsonLeaf(t *testing.T) {
	var size int

	str1 := "12345678"
	size = SizeOfJsonLeaf(str1)
	if size != 8 {
		t.Errorf("Unxpected size for string 1: %d", size)
	}

	str2 := ""
	for i := 0; i < 1024; i++ {
		str2 += "12345678"
	}
	size = SizeOfJsonLeaf(str2)
	if size != 1024*8 {
		t.Errorf("Unxpected size for string 2: %d", size)
	}

	i64 := int64(34)
	size = SizeOfJsonLeaf(i64)
	if size != 8 {
		t.Errorf("Unxpected size for int64: %d", size)
	}

	i32 := int32(128)
	size = SizeOfJsonLeaf(i32)
	if size != 4 {
		t.Errorf("Unxpected size for int32: %d", size)
	}

	f64 := float64(3.223235)
	size = SizeOfJsonLeaf(f64)
	if size != 8 {
		t.Errorf("Unxpected size for float64: %d", size)
	}

	uptr := uintptr(12345)
	size = SizeOfJsonLeaf(uptr)
	if size != 8 {
		t.Errorf("Unxpected size for uintptr: %d", size)
	}

	b := bool(true)
	size = SizeOfJsonLeaf(b)
	if size != 1 {
		t.Errorf("Unxpected size for bool: %d", size)
	}
}

func TestSizeOfString(t *testing.T) {
	var size int

	str1 := "12345678"
	size = SizeOfString(str1)
	if size != 8 {
		t.Errorf("Unxpected size for string 1: %d", size)
	}

	str2 := ""
	for i := 0; i < 1024; i++ {
		str2 += "12345678"
	}
	size = SizeOfString(str2)
	if size != 1024*8 {
		t.Errorf("Unxpected size for string 2: %d", size)
	}
}

func TestSizeOfDirectValue(t *testing.T) {
	var size int

	i64 := int64(34)
	size = SizeOfDirectValue(i64)
	if size != 8 {
		t.Errorf("Unxpected size for int64: %d", size)
	}

	i32 := int32(128)
	size = SizeOfDirectValue(i32)
	if size != 4 {
		t.Errorf("Unxpected size for int32: %d", size)
	}

	f64 := float64(3.223235)
	size = SizeOfDirectValue(f64)
	if size != 8 {
		t.Errorf("Unxpected size for float64: %d", size)
	}

	uptr := uintptr(12345)
	size = SizeOfDirectValue(uptr)
	if size != 8 {
		t.Errorf("Unxpected size for uintptr: %d", size)
	}

	b := bool(true)
	size = SizeOfDirectValue(b)
	if size != 1 {
		t.Errorf("Unxpected size for bool: %d", size)
	}
}
