import { Module } from '@nestjs/common';
import { LayoutsController } from './layouts.controller';
import { LayoutsService } from './layouts.service';
import { LAYOUTS_SERVICE } from './layouts.constants';
import { DBIModule } from 'src/dbi/dbi.module';

@Module({
  imports: [DBIModule],
  controllers: [LayoutsController],
  providers: [
    {
      useClass: LayoutsService,
      provide: LAYOUTS_SERVICE,
    },
  ],
  exports: [LAYOUTS_SERVICE],
})
export class LayoutsModule {}
