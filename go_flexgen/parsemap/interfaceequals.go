package parsemap

import "strconv"

// Casts interface 2 as type of interface 1, then compares them. Compares primitive types only.
func InterfaceEquals(i1 interface{}, i2 interface{}) bool {
	switch i1 := i1.(type) {
	case bool:
		return compareBoolToInterface(i1, i2)
	case int:
		return compareIntToInterface(i1, i2)
	case float32:
		return compareFloat32ToInterface(i1, i2)
	case float64:
		return compareFloat64ToInterface(i1, i2)
	case string:
		return compareStringToInterface(i1, i2)
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given bool
func compareBoolToInterface(b bool, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return b == i
	case int:
		return b == (i != 0)
	case float32:
		return b == (i != 0.0)
	case float64:
		return b == (i != 0.0)
	case string:
		return b == (i != "")
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given int
func compareIntToInterface(n int, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (n == 0 && !i) || (n != 0 && i)
	case int:
		return n == i
	case float32:
		return n == int(i)
	case float64:
		return n == int(i)
	case string:
		iVal, err := strconv.Atoi(i)
		return (err == nil) && (n == iVal)
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given float32
func compareFloat32ToInterface(f float32, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (f == 0.0 && !i) || (f != 0.0 && i)
	case int:
		return f == float32(i)
	case float32:
		return f == i
	case float64:
		return f == float32(i)
	case string:
		iVal, err := strconv.ParseFloat(i, 32)
		return (err == nil) && (f == float32(iVal))
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given float64
func compareFloat64ToInterface(f float64, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (f == 0.0 && !i) || (f != 0.0 && i)
	case int:
		return f == float64(i)
	case float32:
		return f == float64(i)
	case float64:
		return f == i
	case string:
		iVal, err := strconv.ParseFloat(i, 64)
		return (err == nil) && (f == iVal)
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given float64
func compareStringToInterface(s string, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (s == "" && !i) || (s != "" && i)
	case int:
		return s == strconv.Itoa(i)
	case float32:
		// Could use sprintf to convert the interface to string, but then precision leaves ambiguity in string to string comparison
		iVal, err := strconv.ParseFloat(s, 32)
		return (err == nil) && (i == float32(iVal))
	case float64:
		// Could use sprintf to convert the interface to string, but then precision leaves ambiguity in string to string comparison
		iVal, err := strconv.ParseFloat(s, 64)
		return (err == nil) && (i == float64(iVal))
	case string:
		return s == i
	default:
		return false
	}
}
