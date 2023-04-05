import { Injectable } from '@nestjs/common'
import { ThrottlerGuard, ThrottlerException } from '@nestjs/throttler'
import { ExecutionContext } from '@nestjs/common'

@Injectable()
export class HttpThrottlerGuard extends ThrottlerGuard {
    async handleRequest(context: ExecutionContext, limit: number, ttl: number): Promise<boolean> {
        const request = context.switchToHttp().getRequest()
        const ip = request.ip
        const key = this.generateKey(context, ip)
        const ttls = await this.storageService.getRecord(key)

        if (ttls.length == limit + 1) {
            console.log('queueing a message')
            const delayWithPromise = new Promise<boolean>((resolve) => {
                setTimeout(() => {
                    resolve(true)
                }, ttl * 1000)
            })
            return await delayWithPromise
        }

        if (ttls.length > limit + 1) {
            console.log('HttpThrottlerGuard: Throttling limit exceeded')
            throw new ThrottlerException('Too many requests')
        }

        await this.storageService.addRecord(key, ttl)
        return true
    }
}
