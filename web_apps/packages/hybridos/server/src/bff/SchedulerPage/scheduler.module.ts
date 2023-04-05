import { Module } from '@nestjs/common'
import { SchedulerController } from './scheduler.controller'
import { SchedulerGateway } from './scheduler.gateway'
import { SchedulerService } from './scheduler.service'
import { FimsModule } from '../../fims/fims.module'
import { JwtModule } from '@nestjs/jwt'
import { FimsService } from 'src/fims/fims.service'
import { FIMS_SERVICE } from 'src/fims/interfaces/fims.interface'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'

@Module({
    imports: [
        FimsModule,
        JwtModule.register({
        secret: 'supersecretkey',})
    ],
    controllers: [SchedulerController],
    providers: [
        {
            useClass: FimsService,
            provide: FIMS_SERVICE
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
        SchedulerService,
        SchedulerGateway
    ],
})
export class SchedulerModule {}
