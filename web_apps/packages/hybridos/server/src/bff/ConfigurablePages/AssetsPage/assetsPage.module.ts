import { Module } from '@nestjs/common';
import { ConfigModule as CustomConfigModule } from 'src/config/config.module';
import { DBIModule } from 'src/dbi/dbi.module';
import { FimsModule } from '../../../fims/fims.module';
import { AssetsPageGateway } from './assetsPage.gateway';
import { AssetsPageService } from './assetsPage.service';
import { LockModeModule } from './lockMode/lockMode.module';

@Module({
  imports: [CustomConfigModule, FimsModule, DBIModule, LockModeModule],
  controllers: [],
  providers: [AssetsPageService, AssetsPageGateway],
})
export class AssetsPageModule {}
