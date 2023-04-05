import { Module } from '@nestjs/common'
import { FimsModule } from '../../fims/fims.module'
import { VariableOverrideService } from './ercotOverride.service'
import { VariableOverrideController } from './ercotOverride.controller'
import { VariableOverrideGateway } from './ercotOverride.gateway'
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
    controllers: [VariableOverrideController],
    providers: [
        {
            useClass: FimsService,
            provide: FIMS_SERVICE
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
        VariableOverrideService, 
        VariableOverrideGateway
    ],
})
export class VariableOverrideModule {}
