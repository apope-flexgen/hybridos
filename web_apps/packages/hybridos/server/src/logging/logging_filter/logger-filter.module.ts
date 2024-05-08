import { Logger, Module } from '@nestjs/common';
import { LoggerFilter } from './logger-filter.service';

@Module({
  providers: [LoggerFilter],
  exports: [LoggerFilter],
})
export class LoggerFilterModule {}
