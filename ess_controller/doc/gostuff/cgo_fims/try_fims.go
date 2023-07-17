package main

// #cgo LDFLAGS: -L/usr/local/lib -lgofims
// #include <stdio.h>
// #include <stdlib.h>
// #include "gofims.h"
import "C"

import (
	"fmt"
	"unsafe"
)

type Fims struct {
	ptr unsafe.Pointer
}

func NewFims() Fims {
	var foo Fims
	foo.ptr = C.FIMS_NewFims()
	return foo
}

func (foo Fims) ConnectFims(name string) (ret int) {

	ret = int(C.FIMS_ConnectFims(foo.ptr, C.CString(name)))
	return ret
}

func (foo Fims) SendFims(method string, uri string, replyto string, body string) (ret int) {

	ret = int(C.FIMS_SendFims(foo.ptr, C.CString(method), C.CString(uri), C.CString(replyto), C.CString(body)))
	return ret
}

func (foo Fims) SendRawFims(body string, len int) (ret int) {

	ret = int(C.FIMS_SendRawFims(foo.ptr, C.CString(body), C.int(len)))
	return ret
}

func (foo Fims) Free() {
	C.FIMS_DestroyFims(foo.ptr)
}

//func (foo Foo) value() int {
//	return int(C.LIB_FooValue(foo.ptr))
//}

func main() {
	foo := NewFims()
	foo.ConnectFims("/gofims")
	foo.SendRawFims("{\"method\":\"sub\",\"body\":[{ \"uri\":\"/go/fims/test\",\"pub_only\":false}]}", 0)
	foo.SendFims("pub", "/my/go/test", "", "{ \"hello\":\"its a beautiful day\"}")
	defer foo.Free() // The Go analog to C++'s RAII
	fmt.Println("[go] fims ok")
}
