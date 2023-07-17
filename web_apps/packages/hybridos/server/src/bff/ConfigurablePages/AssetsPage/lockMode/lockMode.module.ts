import { Module } from '@nestjs/common';
import { DBIModule } from 'src/dbi/dbi.module';
import { FimsModule } from '../../../../fims/fims.module';
import { LockModeInterceptor } from './../lockMode/lockMode.interceptor';
import { LockModeService } from './../lockMode/lockMode.service';
import { LockModeGateway } from './lockMode.gateway';

@Module({
    imports: [FimsModule, DBIModule],
    controllers: [],
    providers: [LockModeInterceptor, LockModeService, LockModeGateway],
    exports: [LockModeService],
})
export class LockModeModule {}
