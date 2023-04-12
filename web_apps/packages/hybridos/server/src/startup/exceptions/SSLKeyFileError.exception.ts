export class SSLKeyFileError extends Error {
    constructor(message: string) {
        super()
        this.message = message
    }
}
