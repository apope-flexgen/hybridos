import { Global, Module } from '@nestjs/common'
import { ConfigModule } from 'src/config/config.module'
import { AppEnvService } from './appEnv.service'

@Global()
@Module({
    imports:[ConfigModule],
    providers: [AppEnvService],
    exports: [AppEnvService],
})
export class AppEnvModule {}
