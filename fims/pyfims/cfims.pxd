from libcpp cimport bool

cdef extern from "libfims.h":
    cdef cppclass fims:
        fims() except +
        bool Connect(char* process_name)
        void Close()
        bool Subscribe(const char** uri_array, int count, bool* pub_array)
        bool Send(const char* method, const char* uri, const char* replyto, const char* body, const char* username)
        fims_message* Receive()
        fims_message* Receive_Timeout(int useconds)
        bool free_message(fims_message* message)
        const bool Connected() const
        const int get_socket() const

    cdef cppclass fims_message:
        fims_message() except +
        char* method
        char* uri
        char* replyto
        char* body
        unsigned int nfrags
        char** pfrags

cdef extern from "libfims.cpp":
    pass