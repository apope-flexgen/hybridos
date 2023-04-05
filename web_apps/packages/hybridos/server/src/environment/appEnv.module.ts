import { Global, Module } from '@nestjs/common'
import { AppEnvService } from './appEnv.service'

@Global()
@Module({
    controllers: [],
    providers: [AppEnvService],
    exports: [AppEnvService],
})
export class AppEnvModule {}
