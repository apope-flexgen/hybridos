import { SSLValidation } from "./SSLValidation"
import * as fs from 'fs'
import { ConfigPathWebServerError } from "./exceptions/ConfigPathWebServerError.exception"
import { ConfigPathWebUIBuildError } from "./exceptions/ConfigPathWebUIBuildError.exception"
import { ConfigPathWebUIError } from "./exceptions/ConfigPathWebUIError.exception"

export class ProcessArgvValidation {
    webUIConfigPath = undefined
    webServerConfigPath = undefined
    webUIBuildPath = undefined
    ssl = new SSLValidation()
    constructor() {
        this.setWebUIConfigPathArg()
        this.setWebServerConfigPathArg()
        this.setWebUIBuildPathArg()
        this.setSSL()
    }
    setWebUIConfigPathArg() {
        try {
            const configPathArgExists = fs.existsSync(process.argv[3])
            if (!configPathArgExists) {
                throw new ConfigPathWebUIError('Invalid web_ui config path arg')
            }
            this.webUIConfigPath = process.argv[3]
        } catch (e) {
            throw new ConfigPathWebUIError(e)
        }
    }
    setWebServerConfigPathArg() {
        try {
            const configPathArgExists = fs.existsSync(process.argv[4])
            if (!configPathArgExists) {
                throw new ConfigPathWebServerError('Invalid web_server config path arg')
            }
            this.webServerConfigPath = process.argv[4]
        } catch (e) {
            throw new ConfigPathWebServerError(e)
        }
    }
    setWebUIBuildPathArg() {
        try {
            const configPathArgExists = fs.existsSync(process.argv[2])
            if (!configPathArgExists) {
                throw new ConfigPathWebUIBuildError('Invalid web_ui build path arg')
            }
            this.webUIBuildPath = process.argv[2]
        } catch (e) {
            throw new ConfigPathWebUIBuildError(e)
        }
    }
    setSSL() {
        this.ssl.getSSLFiles(this.webUIConfigPath)
    }
}
