import { Global, Module } from '@nestjs/common';
import { DATA_PRUNING_SERVICE } from 'src/dataPruning/dataPruning.interface';
import { DataPruningService } from 'src/dataPruning/dataPruning.service';

@Global()
@Module({
  imports: [],
  controllers: [],
  providers: [{ provide: DATA_PRUNING_SERVICE, useClass: DataPruningService }],
  exports: [DATA_PRUNING_SERVICE],
})
export class DataPruningModule {}
