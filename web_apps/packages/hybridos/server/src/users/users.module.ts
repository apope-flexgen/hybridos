import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { MongooseModule } from '@nestjs/mongoose'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { DBIModule } from 'src/dbi/dbi.module'
import { DBIService } from 'src/dbi/dbi.service'
import { SiteAdminsModule } from '../siteAdmins/siteAdmins.module'
import { DefaultUserService } from './defaultUser.service'
import { DEFAULT_USER_SERVICE } from './interfaces/defaultUser.service.interface'
import { USERS_SERVICE } from './interfaces/users.service.interface'
import { User, UserSchema } from './user.schema'
import { UsersController } from './users.controller'
import { UsersService } from './users.service'
import { ValidPasswordConstraint } from './validators/IsValidPassword'

@Module({
    imports: [
        MongooseModule.forFeature([{ name: User.name, schema: UserSchema }]),
        SiteAdminsModule,
        JwtModule.register({
            secret: 'supersecretkey',
        }),
        DBIModule,
    ],
    controllers: [UsersController],
    providers: [
        { useClass: DefaultUserService, provide: DEFAULT_USER_SERVICE },
        {
            useClass: UsersService,
            provide: USERS_SERVICE,
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
        {
            provide: DBI_SERVICE,
            useClass: DBIService,
        },
        ValidPasswordConstraint,
    ],
    exports: [
        {
            useClass: UsersService,
            provide: USERS_SERVICE,
        },
    ],
})
export class UsersModule {}
