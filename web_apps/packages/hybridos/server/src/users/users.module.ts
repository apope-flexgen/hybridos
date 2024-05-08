import { Module } from '@nestjs/common';
import { APP_GUARD } from '@nestjs/core';
import { MongooseModule } from '@nestjs/mongoose';
import { RolesGuard } from 'src/auth/guards/roles.guard';
import { AuditLoggingModule } from '../logging/auditLogging/auditLogging.module';
import { SiteAdminsModule } from '../siteAdmins/siteAdmins.module';
import { DefaultUserService } from './defaultUser.service';
import { DEFAULT_USER_SERVICE } from './interfaces/defaultUser.service.interface';
import { USERS_SERVICE } from './interfaces/users.service.interface';
import { User, UserSchema } from './user.schema';
import { UsersController } from './users.controller';
import { UsersService } from './users.service';
import { ValidPasswordConstraint } from './validators/IsValidPassword';

@Module({
  imports: [
    MongooseModule.forFeature([{ name: User.name, schema: UserSchema }]),
    SiteAdminsModule,
    AuditLoggingModule,
  ],
  controllers: [UsersController],
  providers: [
    { useClass: DefaultUserService, provide: DEFAULT_USER_SERVICE },
    {
      useClass: UsersService,
      provide: USERS_SERVICE,
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
