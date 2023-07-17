import { Module } from '@nestjs/common';
import { ConfigModule } from 'src/config/config.module';
import {
  PERMISSIONS_FILE_READER_SERVICE,
  PERMISSIONS_SERVICE,
} from './interfaces/permission.interface';
import { PermissionsFileReaderService } from './permissions.fileReader.service';
import { PermissionsServiceFactory } from './permissions.helper';

@Module({
  controllers: [],
  providers: [
    { provide: PERMISSIONS_FILE_READER_SERVICE, useClass: PermissionsFileReaderService },
    {
      provide: PERMISSIONS_SERVICE,
      useFactory: PermissionsServiceFactory,
      inject: [PERMISSIONS_FILE_READER_SERVICE],
    },
  ],
  exports: [PERMISSIONS_SERVICE],
  imports: [ConfigModule],
})
export class PermissionsModule {}
