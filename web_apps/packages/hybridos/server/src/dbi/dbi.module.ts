import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { FimsModule } from 'src/fims/fims.module'
import { FimsService } from 'src/fims/fims.service'
import { FIMS_SERVICE } from 'src/fims/interfaces/fims.interface'
import { DBI_SERVICE } from './dbi.constants'
import { DBIService } from './dbi.service'

@Module({
    imports: [
        FimsModule,
        JwtModule.register({
            secret: 'supersecretkey',
        }),
    ],
    providers: [
        {
            useClass: FimsService,
            provide: FIMS_SERVICE
        },
        {
            useClass: DBIService,
            provide: DBI_SERVICE,
        },
    ],
    exports: [
        {
            useClass: FimsService,
            provide: FIMS_SERVICE
        }
    ],
})

export class DBIModule { }
