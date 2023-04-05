declare const module: any

import { NestFactory } from '@nestjs/core'
import { AuthWsAdapter } from './adapters/authWs.adapter'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'
import * as fs from 'node:fs'
import * as path from 'node:path'

import { AppModule } from './app/app.module'
import { AppEnvService } from './environment/appEnv.service'
import { NestSwagger } from './openapi/nestswagger'
import { RequestMethod } from '@nestjs/common'
import { useContainer } from 'class-validator'

// eslint-disable-next-line max-statements
async function bootstrap() {
    const webUIConfigPath = process.argv[3]
    const configPathArgExists = fs.existsSync(webUIConfigPath)

    let sslPath: string
    if (configPathArgExists) {
        sslPath = path.resolve(webUIConfigPath, 'ssl')
    }
    const keyFilePath = configPathArgExists
        ? path.join(sslPath, 'hybridos-key.pem')
        : './src/certs/key.pem'
    const certFilePath = configPathArgExists
        ? path.join(sslPath, 'hybridos-cert.pem')
        : './src/certs/server.crt'

    const httpsOptions = {
        // key: fs.readFileSync('./src/certs/key.pem'),
        // cert: fs.readFileSync('./src/certs/server.crt'),
        key: fs.readFileSync(keyFilePath),
        cert: fs.readFileSync(certFilePath),
    }
    const app = await NestFactory.create(AppModule, {
        httpsOptions,
    })
    useContainer(app.select(AppModule), { fallbackOnErrors: true })

    const appEnvService = app.get(AppEnvService)
    const PORT = appEnvService.getAppServerPort()

    // Generate OpenAPI Documentation
    const nestSwagger = new NestSwagger(app)
    nestSwagger.generateDocumentation()

    app.use(cookieParser())
    app.useWebSocketAdapter(new AuthWsAdapter(app))
    app.enableCors({
        origin: ['https://172.16.1.80', 'http://localhost'],
        credentials: true,
    })
    app.setGlobalPrefix('api', {
        exclude: [{ path: 'rest', method: RequestMethod.ALL }],
    })
    await app.listen(PORT)
    console.log('nest_web_server listening on port ' + PORT)

    if (module.hot) {
        module.hot.accept()
        module.hot.dispose(() => app.close())
    }
}
bootstrap()
