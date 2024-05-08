import { Module } from '@nestjs/common';
import { FimsModule } from 'src/fims/fims.module';
import { DBI_SERVICE } from './dbi.interface';
import { DBIService } from './dbi.service';

@Module({
  imports: [FimsModule],
  providers: [
    {
      useClass: DBIService,
      provide: DBI_SERVICE,
    },
  ],
  exports: [
    {
      useClass: DBIService,
      provide: DBI_SERVICE,
    },
  ],
})
export class DBIModule {}
