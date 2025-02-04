package fims

import (
	"bytes"
	"crypto/aes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"reflect"
	"unsafe"

	unix "golang.org/x/sys/unix"
)

// unix functions:

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

// non-unix code:

// constants:
const (
	Socket_Name                     = "/tmp/FlexGen_FIMS_Server.socket"
	Aes_Keyfile                     = "/usr/local/etc/config/fims_aes"
	Max_Header_Str_Size             = 255
	Meta_Data_Info_Buf_Len          = 255 * 5
	Max_Expected_Data_len           = 924288 - Meta_Data_Info_Buf_Len - 8 // 8 is sizeof(Meta_Data_Info)
	Fims_Data_Layout_Version uint16 = 1
)

// AES functions:
// source: https://golangdocs.com/aes-encryption-decryption-in-golang

// this is the least efficient verison due to having to convert the string into a bytes array:
func EncryptAES_String(key []byte, plaintext string) ([]byte, error) {
	c, err := aes.NewCipher(key)
	if err != nil {
		return []byte{}, fmt.Errorf("could not create an aes cipher from the given key, err: %v", err)
	}

	out := make([]byte, len(plaintext)+(16-len(plaintext)%16))

	if len(plaintext) < 16 {
		to_send := make([]byte, len(plaintext)+(16-len(plaintext)%16))
		copy(to_send, plaintext)
		c.Encrypt(out, to_send)
	} else {
		c.Encrypt(out, []byte(plaintext))
	}

	return out, nil
}

// this version is more efficient (will probably use this a majority of the time)
func EncryptAES_Bytes(key []byte, plaintext []byte) ([]byte, error) {
	c, err := aes.NewCipher(key)
	if err != nil {
		return []byte{}, fmt.Errorf("could not create an aes cipher from the given key, err: %v", err)
	}

	out := make([]byte, len(plaintext)+(16-len(plaintext)%16))

	if len(plaintext) < 16 {
		to_send := make([]byte, len(plaintext)+(16-len(plaintext)%16))
		copy(to_send, plaintext)
		c.Encrypt(out, to_send)
	} else {
		c.Encrypt(out, plaintext)
	}

	return out, nil
}

func DecryptAES(key []byte, ciphertext []byte) ([]byte, error) {
	c, err := aes.NewCipher(key)
	if err != nil {
		return []byte{}, fmt.Errorf("could not create an aes cipher from the given key, err: %v", err)
	}

	plaintext := make([]byte, len(ciphertext))
	c.Decrypt(plaintext, ciphertext)

	return plaintext, nil
}

// NOTE(WALKER): internal use only
type Handshake struct {
	Fims_data_layout_version uint16
	Max_message_size         uint32
}

// NOTE(WALKER): This struct contains the "offset" information and strlen/Data_len information after receiving data using readv
// aka: "header"
type Meta_Data_Info struct {
	Method_len       uint8
	Uri_len          uint8
	Replyto_len      uint8
	Process_name_len uint8
	Username_len     uint8
	Data_len         uint32
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

// dynamic version:
func (f *Fims) recv_raw_dynamic(bufs *Receiver_Bufs_Dynamic) (bool, error) {
	recv_bufs := []unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(&bufs.Meta_data)), Len: uint64(unsafe.Sizeof(bufs.Meta_data))},
		{Base: &bufs.Data_buf[0], Len: uint64(len(bufs.Data_buf))},
	}
	bytes_read, err := readv(f.fd, recv_bufs)
	return bytes_read > 0, err
}

// Fims connection struct
type Fims struct {
	connected             bool
	fd                    int
	max_expected_Data_len uint32
	name                  string
	aes_key               []byte
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

	sock := unix.SockaddrUnix{Name: Socket_Name}
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
		return Fims{}, fmt.Errorf("err on receiving handshake: %w", err)
	}

	recv_layout_version := handshake.Fims_data_layout_version
	handshake.Fims_data_layout_version = Fims_Data_Layout_Version

	amount_written, err := writev(fd, handshake_buf)
	if amount_written <= 0 || err != nil {
		return Fims{}, fmt.Errorf("err on sending handshake back: %w", err)
	}

	// check for handshake layout version:
	if recv_layout_version != Fims_Data_Layout_Version {
		return Fims{}, fmt.Errorf("server's data layout version was %v instead of the expected version of %v. Cannot continue", recv_layout_version, Fims_Data_Layout_Version)
	}

	// now send the server the process name:
	pName_header := (*reflect.StringHeader)(unsafe.Pointer(&pName))
	name_buf := []unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(pName_header.Data)), Len: uint64(pName_header.Len)},
	}

	amount_written, err = writev(fd, name_buf)
	if amount_written <= 0 || err != nil {
		return Fims{}, fmt.Errorf("err on sending pName to server: %w", err)
	}

	// return socket to blocking
	timeout.Sec = 0
	err = unix.SetsockoptTimeval(fd, unix.SOL_SOCKET, unix.SO_RCVTIMEO, &timeout)
	if err != nil {
		return Fims{}, fmt.Errorf("err on reseting socket timeout: %v", err)
	}

	// grab an aes_key if we have one:
	aes_key_data, err := ioutil.ReadFile(Aes_Keyfile)

	// return:
	if err == nil { // we have an aes_key
		return Fims{connected: true, fd: fd, max_expected_Data_len: handshake.Max_message_size, name: pName, aes_key: aes_key_data}, nil
	} else { // we don't have an aes_key
		return Fims{connected: true, fd: fd, max_expected_Data_len: handshake.Max_message_size, name: pName}, nil
	}
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

	bytes_written, err := new_send_bytes(f.fd, "sub", "", "", f.name, "", to_send.Bytes(), f.aes_key)
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

	data_offset := int(recv_bufs.Meta_data.Method_len) + int(recv_bufs.Meta_data.Uri_len) + int(recv_bufs.Meta_data.Replyto_len) + int(recv_bufs.Meta_data.Process_name_len) + int(recv_bufs.Meta_data.Username_len)
	data := recv_bufs.Data_buf[data_offset : uint32(data_offset)+recv_bufs.Meta_data.Data_len]

	// TODO(WALKER): "decrypt" stuff
	if f.Has_aes_encryption() {
		data, err = DecryptAES(f.aes_key, data)
		if err != nil {
			return fmt.Errorf("could not decrypt data, got: %v", err)
		}
	}

	data_str := string(data)
	// string check (after decryption -> if we have it):
	if data_str != "SUCCESS" {
		return fmt.Errorf("did not get success string for subscribe, got: \"%v\" instead", string(data))
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
		return 0, fmt.Errorf("method exceeds %d characters", Max_Header_Str_Size)
	}
	if len(msg.Uri) > Max_Header_Str_Size {
		return 0, fmt.Errorf("uri exceeds %d characters", Max_Header_Str_Size)
	}
	if len(msg.Replyto) > Max_Header_Str_Size {
		return 0, fmt.Errorf("replyto exceeds %d characters", Max_Header_Str_Size)
	}
	if len(msg.ProcessName) > Max_Header_Str_Size {
		return 0, fmt.Errorf("process_name exceeds %d characters", Max_Header_Str_Size)
	}
	if len(msg.Username) > Max_Header_Str_Size {
		return 0, fmt.Errorf("user name exceeds %d characters", Max_Header_Str_Size)
	}
	// to support custom calls (with no process_name provided):
	if len(msg.ProcessName) == 0 {
		msg.ProcessName = f.name
	}
	// make body a byte array from an interface:
	body_data, err := json.Marshal(msg.Body)
	if err != nil {
		return 0, fmt.Errorf("could not convert \"body\" to a json string, err: %v", err)
	}

	// send it out:
	return new_send_bytes(f.fd, msg.Method, msg.Uri, msg.Replyto, msg.ProcessName, msg.Username, body_data, f.aes_key)
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
		ProcessName: f.name,
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
		ProcessName: f.name,
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
		ProcessName: f.name,
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
		ProcessName: f.name,
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
		ProcessName: f.name,
		Body:        body,
	})
	return err
}

// TODO(WALKER): figure out what "VerificationRecords" is:
func (f *Fims) SendAndVerify(recordType string, msg FimsMsg) (int, error) {
	switch msg.Method {
	case "set", "post":
		added, verifiableReplyto, err := VerificationRecords.addVerificationRecordEntry(recordType, msg)
		if !added {
			return 0, err
		} else if err != nil {
			// Record was added but overwrote an existing entry due to message limit
			log.Println("Overwrote existing entry: ", err)
		}
		msg.Replyto = verifiableReplyto
	case "del":
		return 0, fmt.Errorf("verification of del messages not yet supported")
	}
	return f.Send(msg)
}

func (f *Fims) ResendUnverifiedMessages() error {
	unverifiedMsgs, err := VerificationRecords.GetUnverifiedMessages()
	if err != nil {
		return err
	}

	// Simply resend any messages returned
	for _, entry := range unverifiedMsgs {
		f.Send(entry.msg)
	}

	return nil
}

// Logic to populate data to fims buffer via fims.connection
func (f *Fims) Receive() (FimsMsg, error) {
	var msg FimsMsg

	recv_bufs := Receiver_Bufs_Dynamic{}
	recv_bufs.Data_buf = make([]byte, f.max_expected_Data_len)

	succ, err := f.recv_raw_dynamic(&recv_bufs)
	if !succ {
		f.connected = false
		return FimsMsg{}, fmt.Errorf("error reading fims connection: %v", err)
	}

	// new logic to unpack the buffer into msg:
	curr_idx := 0

	// method (mandatory):
	msg.Method = string(recv_bufs.Data_buf[:recv_bufs.Meta_data.Method_len])
	curr_idx += int(recv_bufs.Meta_data.Method_len)
	// uri (mandatory):
	msg.Uri = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Uri_len)])
	curr_idx += int(recv_bufs.Meta_data.Uri_len)
	// replyto (optional):
	if recv_bufs.Meta_data.Replyto_len > 0 {
		msg.Replyto = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Replyto_len)])
		curr_idx += int(recv_bufs.Meta_data.Replyto_len)
	}
	// process_name (mandatory):
	msg.ProcessName = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Process_name_len)])
	curr_idx += int(recv_bufs.Meta_data.Process_name_len)
	// username (optional):
	if recv_bufs.Meta_data.Username_len > 0 {
		msg.Username = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Username_len)])
		curr_idx += int(recv_bufs.Meta_data.Username_len)
	}

	// data (optional):
	if recv_bufs.Meta_data.Data_len > 0 {
		// decrypt data if we have encryption:
		data := recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Data_len)]
		if f.Has_aes_encryption() {
			data, err = DecryptAES(f.aes_key, data)
			if err != nil {
				return FimsMsg{}, fmt.Errorf("could not decrypt incoming fims message, got: %v", err)
			}
		}

		err := json.Unmarshal(data, &msg.Body)
		if err != nil {
			return FimsMsg{}, fmt.Errorf("failed to unmarshal data: %w", err)
		}
	}

	// Retreieve uri fragments for our message
	msg.Frags, err = GetFrags(msg.Uri)
	if err != nil {
		return FimsMsg{}, fmt.Errorf("error parsing %s into frags: %v", msg.Uri, err)
	}

	// Set our frags back to our message body
	msg.Nfrags = len(msg.Frags)

	// Check if the message is a verification response
	if msg.Method == "set" || msg.Method == "post" {
		go VerificationRecords.handleVerificationResponse(msg)
	}
	return msg, nil
}

func (f *Fims) ReceiveChannel(c chan<- FimsMsg) {
	for f.connected {
		msg, err := f.Receive()
		if err != nil {
			log.Printf("Had an error while receiving on %s: %s\n", msg.Uri, err)
		} else {
			// fmt.Printf("about to put the msg on the channel %v\n", msg)
			c <- msg
		}
	}
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

// tells the user if they are using aes_encryption or not:
func (f *Fims) Has_aes_encryption() bool {
	return len(f.aes_key) > 0
}

// new send for []byte array as opposed to strings (this is more generic -> used for subscribe):
func new_send_bytes(fd int, method string, uri string, replyto string, process_name string, username string, data []byte, key []byte) (int, error) {
	meta_data := Meta_Data_Info{}
	send_bufs := [7]unix.Iovec{
		{Base: (*byte)(unsafe.Pointer(&meta_data)), Len: uint64(unsafe.Sizeof(meta_data))},
	}
	send_amount := 1

	// headers:
	method_header := (*reflect.StringHeader)(unsafe.Pointer(&method))
	uri_header := (*reflect.StringHeader)(unsafe.Pointer(&uri))
	replyto_header := (*reflect.StringHeader)(unsafe.Pointer(&replyto))
	process_name_header := (*reflect.StringHeader)(unsafe.Pointer(&process_name))
	username_header := (*reflect.StringHeader)(unsafe.Pointer(&username))

	// for aes_encryption if we have it:
	var new_data []byte

	// method:
	if method_header.Len > 0 {
		meta_data.Method_len = uint8(method_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(method_header.Data)), Len: uint64(meta_data.Method_len)}
		send_amount += 1
	}
	// uri:
	if uri_header.Len > 0 {
		meta_data.Uri_len = uint8(uri_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(uri_header.Data)), Len: uint64(meta_data.Uri_len)}
		send_amount += 1
	}
	// replyto:
	if replyto_header.Len > 0 {
		meta_data.Replyto_len = uint8(replyto_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(replyto_header.Data)), Len: uint64(meta_data.Replyto_len)}
		send_amount += 1
	}
	// process_name:
	if process_name_header.Len > 0 {
		meta_data.Process_name_len = uint8(process_name_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(process_name_header.Data)), Len: uint64(meta_data.Process_name_len)}
		send_amount += 1
	}
	// username:
	if username_header.Len > 0 {
		meta_data.Username_len = uint8(username_header.Len)
		send_bufs[send_amount] = unix.Iovec{Base: (*byte)(unsafe.Pointer(username_header.Data)), Len: uint64(meta_data.Username_len)}
		send_amount += 1
	}
	// body ("data"):
	if len(data) > 0 {
		// if we have "aes_encryption" then send out the encrypted buffer instead:
		if len(key) > 0 {
			aes_data, err := EncryptAES_Bytes(key, data)
			if err != nil {
				return 0, fmt.Errorf("could not encrypt data with the given key, got: %v", err)
			}
			new_data = aes_data
			meta_data.Data_len = uint32(len(new_data))
			send_bufs[send_amount] = unix.Iovec{Base: &new_data[0], Len: uint64(meta_data.Data_len)}
		} else { // no encryption:

			new_data = data
			meta_data.Data_len = uint32(len(data))
			send_bufs[send_amount] = unix.Iovec{Base: &new_data[0], Len: uint64(meta_data.Data_len)}
		}
		send_amount += 1
	}

	return writev(fd, send_bufs[:send_amount])
}

func (f *Fims) ReceiveRawChannel(c chan<- FimsMsg) {
	for f.connected {
		msg, err := f.ReceiveRaw()
		if err != nil {
			log.Printf("Had an error while receiving on %s: %s\n", msg.Uri, err)
		} else {
			// fmt.Printf("about to put the msg on the channel %v\n", msg)
			c <- msg
		}
	}
}


// Logic to populate data to fims buffer via fims.connection
func (f *Fims) ReceiveRaw() (FimsMsg, error) {
	var msg FimsMsg

	recv_bufs := Receiver_Bufs_Dynamic{}
	recv_bufs.Data_buf = make([]byte, f.max_expected_Data_len)

	succ, err := f.recv_raw_dynamic(&recv_bufs)
	if !succ {
		f.connected = false
		return FimsMsg{}, fmt.Errorf("error reading fims connection: %v", err)
	}

	// new logic to unpack the buffer into msg:
	curr_idx := 0

	// method (mandatory):
	msg.Method = string(recv_bufs.Data_buf[:recv_bufs.Meta_data.Method_len])
	curr_idx += int(recv_bufs.Meta_data.Method_len)
	// uri (mandatory):
	msg.Uri = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Uri_len)])
	curr_idx += int(recv_bufs.Meta_data.Uri_len)
	// replyto (optional):
	if recv_bufs.Meta_data.Replyto_len > 0 {
		msg.Replyto = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Replyto_len)])
		curr_idx += int(recv_bufs.Meta_data.Replyto_len)
	}
	// process_name (mandatory):
	msg.ProcessName = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Process_name_len)])
	curr_idx += int(recv_bufs.Meta_data.Process_name_len)
	// username (optional):
	if recv_bufs.Meta_data.Username_len > 0 {
		msg.Username = string(recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Username_len)])
		curr_idx += int(recv_bufs.Meta_data.Username_len)
	}

	// data (optional):
	if recv_bufs.Meta_data.Data_len > 0 {
		// decrypt data if we have encryption:
		data := recv_bufs.Data_buf[curr_idx : curr_idx+int(recv_bufs.Meta_data.Data_len)]
		if f.Has_aes_encryption() {
			data, err = DecryptAES(f.aes_key, data)
			if err != nil {
				return FimsMsg{}, fmt.Errorf("could not decrypt incoming fims message, got: %v", err)
			}
		}
		msg.Body = data
		// err := json.Unmarshal(data, &msg.Body)
		// if err != nil {
		// 	return FimsMsg{}, fmt.Errorf("failed to unmarshal data: %w", err)
		// }
	}

	// Retreieve uri fragments for our message
	msg.Frags, err = GetFrags(msg.Uri)
	if err != nil {
		return FimsMsg{}, fmt.Errorf("error parsing %s into frags: %v", msg.Uri, err)
	}

	// Set our frags back to our message body
	msg.Nfrags = len(msg.Frags)

	// Check if the message is a verification response
	//if msg.Method == "set" || msg.Method == "post" {
	//	go VerificationRecords.handleVerificationResponse(msg)
	//}
	return msg, nil
}

