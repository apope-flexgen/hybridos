package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"
	"unsafe"

	unix "golang.org/x/sys/unix"
)

// constants:
const (
	SocketName                      = "/tmp/FlexGen_FIMS_Server.socket"
	Max_Header_Str_Size             = 255
	Meta_Data_Info_Buf_Len          = 255 * 4
	Max_Expected_Data_len           = 924288 - Meta_Data_Info_Buf_Len - 8 // 8 is sizeof(Meta_Data_Info)
	Fims_Data_Layout_Version uint16 = 1
)

type Handshake struct {
	Fims_data_layout_version uint16
	Max_message_size         uint32
}

// NOTE(WALKER): This struct contains the "offset" information and strlen/Data_len information after receiving data
// using readv
// aka: "header"
type Meta_Data_Info struct {
	Method_len      uint8
	Uri_len         uint8
	Replyto_len     uint8
	Sender_name_len uint8
	Data_len        uint32
}

// alternatively Data_buf can just be a slice of bytes
// instead of a set size array (void* and len into it can still be used with readv)
type Receiver_Bufs_Static struct {
	Meta_data Meta_Data_Info
	Data_buf  [Meta_Data_Info_Buf_Len + Max_Expected_Data_len]byte
}

// non set-size receiver bufs (after calling connect):
// this will allocate the max_bytes during runtime for receiving (after the handshake from the server):
type Receiver_Bufs_Dynamic struct {
	Meta_data Meta_Data_Info
	Data_buf  []byte
}

// helper function for recv_buff_x receiving:
func (f *Fims) recv_raw_static(bufs *Receiver_Bufs_Static) (bool, error) {
	recv_bufs := []unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(&bufs.Meta_data)), Len: uint64(unsafe.Sizeof(bufs.Meta_data))},
		{Base: &bufs.Data_buf[0], Len: uint64(len(bufs.Data_buf))},
	}
	bytes_read, err := readv(f.fd, recv_bufs)
	return bytes_read > 0, err
}

// Single-word zero for use when we need a valid pointer to 0 bytes.
var _zero uintptr

// Do the interface allocations only once for common
// Errno values.
var (
	errEAGAIN error = unix.EAGAIN
	errEINVAL error = unix.EINVAL
	errENOENT error = unix.ENOENT
)

// errnoErr returns common boxed Errno values, to prevent
// allocations at runtime.
func errnoErr(e unix.Errno) error {
	switch e {
	case 0:
		return nil
	case unix.EAGAIN:
		return errEAGAIN
	case unix.EINVAL:
		return errEINVAL
	case unix.ENOENT:
		return errENOENT
	}
	return e
}

// readv:
func readv(fd int, iovs []unix.Iovec) (n int, err error) {
	var _p0 unsafe.Pointer
	if len(iovs) > 0 {
		_p0 = unsafe.Pointer(&iovs[0])
	} else {
		_p0 = unsafe.Pointer(&_zero)
	}
	r0, _, e1 := unix.Syscall(unix.SYS_READV, uintptr(fd), uintptr(_p0), uintptr(len(iovs)))
	n = int(r0)
	if e1 != 0 {
		err = errnoErr(e1)
	}
	return
}

// writev:
func writev(fd int, iovs []unix.Iovec) (n int, err error) {
	var _p0 unsafe.Pointer
	if len(iovs) > 0 {
		_p0 = unsafe.Pointer(&iovs[0])
	} else {
		_p0 = unsafe.Pointer(&_zero)
	}
	r0, _, e1 := unix.Syscall(unix.SYS_WRITEV, uintptr(fd), uintptr(_p0), uintptr(len(iovs)))
	n = int(r0)
	if e1 != 0 {
		err = errnoErr(e1)
	}
	return
}

// Fims connection struct
type Fims struct {
	connected             bool
	fd                    int
	max_expected_Data_len uint32
	name                  string
}

// FimsMsg struct:
type FimsMsg struct {
	Method      string
	Uri         string
	Replyto     string
	Sender_name string
	Body        interface{}
	Nfrags      int // NOTE(WALKER): why is this necessary? -> "Frags" already has the number of frags in golang (i.e. len(Frags))
	Frags       []string
}

func Connect(pName string) (Fims, error) {
	const Max_Name_Len = 255

	// pName size check:
	if len(pName) > Max_Name_Len {
		return Fims{}, fmt.Errorf("pName (currently: \"%v\") is more than 255 characters", pName)
	}

	fd, err := unix.Socket(unix.AF_UNIX, unix.SOCK_SEQPACKET, 0)
	if err != nil {
		return Fims{}, fmt.Errorf("err on socket creation: %v", err)
	}

	sock := unix.SockaddrUnix{Name: SocketName}
	err = unix.Connect(fd, &sock)
	if err != nil {
		return Fims{}, fmt.Errorf("err on connect syscall: %v", err)
	}

	// set socket size to the appropriate size (no erroring out if we can't):
	sndBufferSize, _ := unix.GetsockoptInt(fd, unix.SOL_SOCKET, unix.SO_SNDBUF)
	if sndBufferSize < Max_Expected_Data_len+Meta_Data_Info_Buf_Len {
		err = unix.SetsockoptInt(fd, unix.SOL_SOCKET, unix.SO_SNDBUF, Max_Expected_Data_len+Meta_Data_Info_Buf_Len)
		if err != nil {
			// just print the socket size change error out (no return apparently):
			fmt.Printf("Failed to set the socket send bufer as %v for sock %v, err: \"%v\"\n", Max_Expected_Data_len+Meta_Data_Info_Buf_Len, fd, err)
		}
	}

	// set socket timeout to be 2 seconds:
	timeout := unix.Timeval{Sec: 2, Usec: 0}
	err = unix.SetsockoptTimeval(fd, unix.SOL_SOCKET, unix.SO_RCVTIMEO, &timeout)
	if err != nil {
		return Fims{}, fmt.Errorf("error on setting socket timeout: %v", err)
	}

	// commence handshake:
	handshake := Handshake{}

	handshake_buf := []unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(&handshake)), Len: uint64(unsafe.Sizeof(handshake))},
	}

	amount_read, err := readv(fd, handshake_buf)
	if amount_read <= 0 || err != nil {
		return Fims{}, fmt.Errorf("err on receiving handshake: %v", err)
	}

	recv_layout_version := handshake.Fims_data_layout_version
	handshake.Fims_data_layout_version = Fims_Data_Layout_Version

	amount_written, err := writev(fd, handshake_buf)
	if amount_written <= 0 || err != nil {
		return Fims{}, fmt.Errorf("err on sending handshake back: %w", err)
	}

	// check for handshake layout version:
	if recv_layout_version != Fims_Data_Layout_Version {
		return Fims{}, fmt.Errorf("server's data layout version was %d instead of the expected version of %d. Cannot continue", recv_layout_version, Fims_Data_Layout_Version)
	}

	// now send the server the process name:
	pName_header := (*reflect.StringHeader)(unsafe.Pointer(&pName))
	name_buf := []unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(pName_header.Data + 0)), Len: uint64(pName_header.Len)},
	}

	amount_written, err = writev(fd, name_buf)
	if amount_written <= 0 || err != nil {
		return Fims{}, fmt.Errorf("err on sending pName to server: %v", err)
	}

	// return socket to blocking
	timeout.Sec = 0
	err = unix.SetsockoptTimeval(fd, unix.SOL_SOCKET, unix.SO_RCVTIMEO, &timeout)
	if err != nil {
		return Fims{}, fmt.Errorf("err on reseting socket timeout: %v", err)
	}

	return Fims{connected: true, fd: fd, max_expected_Data_len: handshake.Max_message_size, name: pName}, nil
}

// for sub only (internal use):
type Sub_Info struct {
	Pub_only bool
	Str_size uint8
}

// with pub_only set to false:
func (f *Fims) Subscribe(uris ...string) error {

	return f.internalSubscribe(false, uris)
}

// with pub_only set to true:
func (f *Fims) SubscribetoPubs(uris ...string) error {
	return f.internalSubscribe(true, uris)
}

// internal sub method that the two above use:
func (f *Fims) internalSubscribe(pub_only bool, uris []string) error {
	const Max_uri_size = 255
	const Max_subs = 64
	Endianness := binary.LittleEndian

	// NOTE(WALKER): make sure that
	if len(uris) > Max_subs {
		return fmt.Errorf("the number of subs: %v is greater than the maximum of %v", len(uris), Max_subs)
	}

	curr_sub_info := Sub_Info{Pub_only: pub_only, Str_size: 0}

	// check that the size of each uri is no more than 255 characters:
	for _, uri := range uris {
		Uri_len := len(uri)
		if Uri_len > Max_uri_size {
			return fmt.Errorf("uri \"%v\" is more than %v characters long", uri, Max_uri_size)
		}
	}

	// send buffer:
	var to_send bytes.Buffer
	binary.Write(&to_send, Endianness, uint8(len(uris)))

	// write in sub_info structs:
	for _, uri := range uris {
		curr_sub_info.Str_size = uint8(len(uri))
		binary.Write(&to_send, Endianness, curr_sub_info)
	}

	// write in strings themselves:
	for _, uri := range uris {
		to_send.WriteString(uri)
	}

	bytes_written, err := new_send_bytes(f.fd, "sub", "", "", f.name, to_send.Bytes())
	if bytes_written <= 0 || err != nil {
		return fmt.Errorf("could not send subscribe to server, err: %v", err)
	}

	// next check for server confirmation message:

	// set timeout to be 2 seconds:
	timeout := unix.Timeval{Sec: 2, Usec: 0}
	// return socket to blocking
	err = unix.SetsockoptTimeval(f.fd, unix.SOL_SOCKET, unix.SO_RCVTIMEO, &timeout)
	if err != nil {
		return fmt.Errorf("err on setting socket timeout: %v", err)
	}

	// confirmation message:
	recv_bufs := Receiver_Bufs_Static{}
	succ, err := f.recv_raw_static(&recv_bufs)
	if !succ {
		return fmt.Errorf("err on receiving confirmation message from the server: %v", err)
	}

	// size check:
	if recv_bufs.Meta_data.Data_len > uint32(len(recv_bufs.Data_buf)-Meta_Data_Info_Buf_Len) {
		return fmt.Errorf("received more bytes than the receive buffer can handle")
	}

	// TODO(WALKER): "decrypt" stuff

	// string check (after decryption):
	data_offset := int(recv_bufs.Meta_data.Method_len) + int(recv_bufs.Meta_data.Uri_len) + int(recv_bufs.Meta_data.Replyto_len) + int(recv_bufs.Meta_data.Sender_name_len)
	data_str := string(recv_bufs.Data_buf[data_offset : uint32(data_offset)+recv_bufs.Meta_data.Data_len])
	if data_str != "SUCCESS" {
		return fmt.Errorf("did not get success string for subscribe, got: \"%v\" instead", data_str)
	}

	// return socket to blocking
	timeout.Sec = 0
	err = unix.SetsockoptTimeval(f.fd, unix.SOL_SOCKET, unix.SO_RCVTIMEO, &timeout)
	if err != nil {
		return fmt.Errorf("err on reseting socket timeout: %v", err)
	}

	return nil
}

// send stuff:
func (f *Fims) Send(msg FimsMsg) (int, error) {
	// length checks:
	if len(msg.Method) > Max_Header_Str_Size {
		return 0, fmt.Errorf("method exceeds %s characters", msg.Method)
	}
	if len(msg.Uri) > Max_Header_Str_Size {
		return 0, fmt.Errorf("uri exceeds %s characters", msg.Uri)
	}
	if len(msg.Replyto) > Max_Header_Str_Size {
		return 0, fmt.Errorf("replyto exceeds %s characters", msg.Replyto)
	}
	if len(msg.Sender_name) > Max_Header_Str_Size {
		return 0, fmt.Errorf("sender_name exceeds %s characters", msg.Sender_name)
	}
	// make body a string from an interface:
	body_str := fmt.Sprint(msg.Body)
	// send it out:
	return new_send_string(f.fd, msg.Method, msg.Uri, msg.Replyto, msg.Sender_name, body_str)
}

// SendSet sends a SET message via FIMS
func (f *Fims) SendSet(uri, replyTo string, body interface{}) error {
	if f == nil {
		return fmt.Errorf("pointer to FIMS is nil")
	}
	_, err := f.Send(FimsMsg{
		Method:      "set",
		Uri:         uri,
		Body:        body,
		Replyto:     replyTo,
		Sender_name: f.name,
	})
	return err
}

// SendGet sends a GET message via FIMS
func (f *Fims) SendGet(uri, replyTo string) error {
	if f == nil {
		return fmt.Errorf("pointer to FIMS is nil")
	}
	_, err := f.Send(FimsMsg{
		Method:      "get",
		Uri:         uri,
		Replyto:     replyTo,
		Sender_name: f.name,
	})
	return err
}

// SendPost sends a POST message via FIMS
func (f *Fims) SendPost(uri, replyTo string, body interface{}) error {
	if f == nil {
		return fmt.Errorf("pointer to FIMS is nil")
	}
	_, err := f.Send(FimsMsg{
		Method:      "post",
		Uri:         uri,
		Body:        body,
		Replyto:     replyTo,
		Sender_name: f.name,
	})
	return err
}

// SendDel sends a DEL message via FIMS
func (f *Fims) SendDel(uri, replyTo string, body interface{}) error {
	if f == nil {
		return fmt.Errorf("pointer to FIMS is nil")
	}
	_, err := f.Send(FimsMsg{
		Method:      "del",
		Uri:         uri,
		Body:        body,
		Replyto:     replyTo,
		Sender_name: f.name,
	})
	return err
}

// SendPub sends a PUB message via FIMS
func (f *Fims) SendPub(uri string, body interface{}) error {
	if f == nil {
		return fmt.Errorf("pointer to FIMS is nil")
	}
	_, err := f.Send(FimsMsg{
		Method:      "pub",
		Uri:         uri,
		Body:        body,
		Sender_name: f.name,
	})
	return err
}

// new Close:
func (f *Fims) Close() error {
	f.connected = false
	err := unix.Close(f.fd)
	if err != nil {
		return fmt.Errorf("error closing fims connection: %v", err)
	}

	return nil
}

// new connected method:
func (f *Fims) Connected() bool {
	return f.connected
}

// TODO(WALKER): get "has_aes_encryption" function in here
// I don't know how to do that in this language

// new fims_send function (base level):
// NOTE(WALKER): "body" is "data" -> for now it is just a string instead of "bytes"
// TODO(WALKER): get "aes_encryption" in here as well (that will be a separate function for encrypting "body"/"data")
func new_send_string(fd int, method string, uri string, replyto string, sender_name string, body string) (int, error) {
	meta_data := Meta_Data_Info{}
	send_bufs := [6]unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(&meta_data)), Len: uint64(unsafe.Sizeof(meta_data))},
	}
	send_amount := 1

	// headers:
	method_header := (*reflect.StringHeader)(unsafe.Pointer(&method))
	uri_header := (*reflect.StringHeader)(unsafe.Pointer(&uri))
	replyto_header := (*reflect.StringHeader)(unsafe.Pointer(&replyto))
	sender_name_header := (*reflect.StringHeader)(unsafe.Pointer(&sender_name))
	body_header := (*reflect.StringHeader)(unsafe.Pointer(&body))

	// method:
	if method_header.Len > 0 {
		meta_data.Method_len = uint8(method_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(method_header.Data + 0)), Len: uint64(meta_data.Method_len)}
		send_amount += 1
	}
	// uri:
	if uri_header.Len > 0 {
		meta_data.Uri_len = uint8(uri_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(uri_header.Data + 0)), Len: uint64(meta_data.Uri_len)}
		send_amount += 1
	}
	// replyto:
	if replyto_header.Len > 0 {
		meta_data.Replyto_len = uint8(replyto_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(replyto_header.Data + 0)), Len: uint64(meta_data.Replyto_len)}
		send_amount += 1
	}
	// sender_name:
	if method_header.Len > 0 {
		meta_data.Sender_name_len = uint8(sender_name_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(sender_name_header.Data + 0)), Len: uint64(meta_data.Sender_name_len)}
		send_amount += 1
	}
	// body ("data"):
	if body_header.Len > 0 {
		meta_data.Data_len = uint32(body_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(body_header.Data + 0)), Len: uint64(meta_data.Data_len)}
		send_amount += 1
	}

	return writev(fd, send_bufs[:send_amount])
}

// new send for []byte array as opposed to strings (this is more generic -> used for subscribe):
func new_send_bytes(fd int, method string, uri string, replyto string, sender_name string, data []byte) (int, error) {
	meta_data := Meta_Data_Info{}
	send_bufs := [6]unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(&meta_data)), Len: uint64(unsafe.Sizeof(meta_data))},
	}
	send_amount := 1

	// headers:
	method_header := (*reflect.StringHeader)(unsafe.Pointer(&method))
	uri_header := (*reflect.StringHeader)(unsafe.Pointer(&uri))
	replyto_header := (*reflect.StringHeader)(unsafe.Pointer(&replyto))
	sender_name_header := (*reflect.StringHeader)(unsafe.Pointer(&sender_name))

	// method:
	if method_header.Len > 0 {
		meta_data.Method_len = uint8(method_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(method_header.Data + 0)), Len: uint64(meta_data.Method_len)}
		send_amount += 1
	}
	// uri:
	if uri_header.Len > 0 {
		meta_data.Uri_len = uint8(uri_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(uri_header.Data + 0)), Len: uint64(meta_data.Uri_len)}
		send_amount += 1
	}
	// replyto:
	if replyto_header.Len > 0 {
		meta_data.Replyto_len = uint8(replyto_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(replyto_header.Data + 0)), Len: uint64(meta_data.Replyto_len)}
		send_amount += 1
	}
	// sender_name:
	if method_header.Len > 0 {
		meta_data.Sender_name_len = uint8(sender_name_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(sender_name_header.Data + 0)), Len: uint64(meta_data.Sender_name_len)}
		send_amount += 1
	}
	// body ("data"):
	if len(data) > 0 {
		meta_data.Data_len = uint32(len(data))
		send_bufs[send_amount] = unix.Iovec{Base: &data[0], Len: uint64(meta_data.Data_len)}
		send_amount += 1
	}

	return writev(fd, send_bufs[:send_amount])
}

// a quick example:
func main() {

	var pName string = "hello world from golang"

	fims, err := Connect(pName)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer fims.Close()

	err = fims.Subscribe("/hello", "/world")
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println("sub and everything is fine, yay!")
}
