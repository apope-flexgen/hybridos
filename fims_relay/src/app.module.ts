import { Module } from '@nestjs/common';
import { RelayModule } from './relay/relay.module';
import { ApiModule } from './api/api.module';
import { APP_INTERCEPTOR } from '@nestjs/core/constants';
import { TimeoutInterceptor } from './interceptors/timeout.interceptor';

@Module({
  imports: [
    RelayModule,
    ApiModule,
  ],
  controllers: [],
  providers: [
    {
      provide: APP_INTERCEPTOR,
      useFactory: () => {
          return new TimeoutInterceptor();
      },
      inject: [],
    },
  ],
})
export class AppModule {}
