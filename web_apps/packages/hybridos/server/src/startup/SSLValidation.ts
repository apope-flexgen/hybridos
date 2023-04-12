import * as fs from 'node:fs'
import * as path from 'node:path'
import { SSLCertFileError } from './exceptions/SSLCertFileError.exception'
import { SSLKeyFileError } from './exceptions/SSLKeyFileError.exception'

const HYBRIDOS_KEY = 'hybridos-key.pem'
const HYBRIDOS_CERT = 'hybridos-cert.pem'
const SSL_DIR_NAME = 'ssl'

export class SSLValidation {
    sslPath = undefined
    keyFile = undefined
    certFile = undefined
    getSSLFiles(webUiConfigPath: string) {
        this.sslPath = path.resolve(webUiConfigPath, SSL_DIR_NAME)
        this.setKeyFilePath()
        this.setCertFilePath()
    }
    setKeyFilePath() {
        try {
            this.keyFile = fs.readFileSync(path.join(this.sslPath, HYBRIDOS_KEY))
        } catch (e) {
            throw new SSLKeyFileError(`Error reading ssl key file: ${e}`)
        }
    }
    setCertFilePath() {
        try {
            this.certFile = fs.readFileSync(path.join(this.sslPath, HYBRIDOS_CERT))
        } catch (e) {
            throw new SSLCertFileError(`Error reading ssl cert file: ${e}`)
        }
    }
}
