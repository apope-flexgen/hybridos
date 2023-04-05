import { INestApplication } from '@nestjs/common'
import { JwtService } from '@nestjs/jwt'
import { WsAdapter } from '@nestjs/platform-ws'
import * as http from 'http'
import { AppEnvService } from '../environment/appEnv.service'
import {
    ISocketAuthService,
    SOCKET_AUTH_SERVICE,
} from '../fims/interfaces/socketAuth.service.interface'

export class AuthWsAdapter extends WsAdapter {
    socketAuthService: ISocketAuthService
    jwtService: JwtService
    appEnvService: AppEnvService

    constructor(app: INestApplication) {
        super(app)

        this.socketAuthService = app.get(SOCKET_AUTH_SERVICE)
        this.jwtService = app.get(JwtService)
        this.appEnvService = app.get(AppEnvService)
    }

    protected ensureHttpServerExists(port: number, httpServer = http.createServer()) {
        httpServer.on('upgrade', (request, socket: any, head) => {
            const token = request.url.split('=')[1]

            let decoded: { role: string, sub: string }
            try {
                decoded = this.jwtService.verify(token, {
                    secret: this.appEnvService.getAccessTokenSecretFimsSocket(),
                })
            } catch (e) {
                socket.destroy()
                return
            }

            socket.isValid = true
            socket.userRole = decoded.role
            socket.username = decoded.sub

            if (!this.socketAuthService.validateToken(token)) {
                socket.destroy()
                return
            }
        })
        return super.ensureHttpServerExists(port, httpServer)
    }
}
