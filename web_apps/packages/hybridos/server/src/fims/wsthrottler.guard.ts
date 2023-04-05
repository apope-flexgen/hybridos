import { Injectable } from '@nestjs/common'
import { ThrottlerGuard } from '@nestjs/throttler'
import { ExecutionContext } from '@nestjs/common'
import { WsException } from '@nestjs/websockets'

@Injectable()
export class WsThrottlerGuard extends ThrottlerGuard {
    async handleRequest(context: ExecutionContext, limit: number, ttl: number): Promise<boolean> {
        const client = context.switchToWs().getClient()
        // this is a generic method to switch between `ws` and `socket.io`. You can choose what is appropriate for you
        const ip = client._socket.remoteAddress
        const key = this.generateKey(context, ip)
        const ttls = await this.storageService.getRecord(key)

        if (ttls.length >= limit) {
            console.log('WsThrottlerGuard: Throttling limit exceeded')
            throw new WsException('Too many requests')
        }

        await this.storageService.addRecord(key, ttl)
        return true
    }
}
