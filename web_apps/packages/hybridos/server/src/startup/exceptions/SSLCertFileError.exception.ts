export  class SSLCertFileError extends Error {
    constructor(message: string) {
        super()
        this.message = message
    }
}
